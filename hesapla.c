#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>



static char msg[20];
static char bosluk[20] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};

//karakterin rakam olup olmadığını kontrol eder rakamsa 1 döndürür değilse 0 döndürür
int isNumber(char c){

    int ret = 0;
    char nums[10] = {'0','1','2','3','4','5','6','7','8','9'};
    int i;
    for(i = 0; i< 10; i++){
        if(c == nums[i]){
            ret = 1;
        }
    }

    return ret;

}

//4 işlem yapabilen fonksiyon
void islem(char i,int iprm1,int iprm2){
    
    //değişkenleri initialize et
	char *sonuc2 = (char*)kmalloc(sizeof(char)*20,GFP_KERNEL);  
    strcpy(sonuc2,bosluk);
    long tempi = 0;
    size_t leni = 0;  

    //i nin işaretine göre işlem yapar
	if(i == '+'){
        tempi = iprm1 + iprm2;
	}else if(i == '-'){
        tempi = iprm1 - iprm2;
	}else if(i == '/'){
        if(iprm2 != 0)
            tempi = iprm1 / iprm2;
        else
            tempi = 0;// eğer bölen 0 ise hata oluşmaması için sıfıra eşitliyoruz
	}else if(i == '*'){
        tempi = iprm1 * iprm2;
	}else{
        strcpy(sonuc2,"HATA");
	}

    //herşey yolunda giderse sonucu buffera al
    if(sonuc2 != "HATA"){
        sprintf(sonuc2,"%d %c %d = %d\n",iprm1,i,iprm2,tempi);
    }
    
    //değişkenleri sıfırla
    leni = strlen(sonuc2);
    memcpy(msg,bosluk,20);
    memcpy(msg,sonuc2,leni);
    
    tempi = 0;
    leni = 0;  
    strcpy(sonuc2,bosluk);
	
    kfree(sonuc2);
}

//alınan stringi parse edip işleyen fonksiyon
void parser(char *take){

  //değişkenleri initialize etme
  char *sonuc1 = (char*)kmalloc(sizeof(char)*20,GFP_KERNEL);
  char *tempp1 = (char*)kmalloc(sizeof(char)*20,GFP_KERNEL);
  char *tempp2 = (char*)kmalloc(sizeof(char)*20,GFP_KERNEL);
  char *param1 = (char*)kmalloc(sizeof(char)*20,GFP_KERNEL);
  char *param2 = (char*)kmalloc(sizeof(char)*20,GFP_KERNEL);
  char *tutp = (char*)kmalloc(sizeof(char)*20,GFP_KERNEL);
  
  strcpy(sonuc1,"basarili");
  strcpy(param1,"");
  strcpy(param2,"");
  strcpy(tempp1,"");
  strcpy(tempp2,"");
  strcpy(tutp,"");
  char op;

    
  int len = strlen(take);
  int sec = 1;
  char c;

  if(len < 1)
    strcpy(sonuc1,"HATA");
  
    //parse stringi parse etmeye başlar ilk sayıyı param1 e 2.sayısı param2 ye işlemi de op değişkenine atar
	int lop;
	for(lop = 0; lop < len; lop++){
		c = take[lop];
        
           if(c == '+' || c == '-' || c == '*' || c == '/' )
               sec = 0;

           if(sec == 1 && isNumber(c)){
               sprintf(tempp1,"%s%c",param1,c);
               strcpy(param1,tempp1);
               strcpy(tempp1,"");
           }else if(sec == 2 && isNumber(c)){
               sprintf(tempp2,"%s%c",param2,c);
               strcpy(param2,tempp2);
               strcpy(tempp2,"");
           }else if(sec == 0){
               op = c;
               sec = 2;
               strcpy(tempp1,"");
               strcpy(tempp2,"");
           }else
               strcpy(sonuc1,"HATA");

        
  }
  
  int prm1 = 0;
  int prm2 = 0;
  long res1 = 0;
  long res2 = 0;
   
  //hata almadıysak param1 ve param2 yi inte çevirerek işlemin yapılması için gerekli fonksiyona parametre olarak yollar
  if(sonuc1 != "HATA"){
    kstrtol(param1,10,&res1);
    prm1 = (int)res1;
    kstrtol(param2,10,&res2);
    prm2 = (int)res2;    
    islem(op,prm1,prm2);  
  }

    //değişkenleri sıfırlama
    strcpy(sonuc1,"");
    strcpy(tempp1,"");
    strcpy(tempp2,"");   
    strcpy(param1,"");
    strcpy(param2,""); 

    prm1 = 0;
    prm2 = 0;
    res1 = 0;
    res2 = 0;

    
    kfree(param1);
    kfree(param2);
    kfree(tempp1);
    kfree(tempp2);
	kfree(sonuc1);

}

//cat ile procdaki veriyi okuma
ssize_t my_proc_read(struct file *filp, char __user *buffer, size_t len, loff_t *offset){


	ssize_t ret;
	size_t param_len = strlen(msg);

	ret = simple_read_from_buffer(buffer, len, offset,msg,param_len);

	return ret;
}

//echo ile proc a parametre yollama
static ssize_t my_proc_write(struct file *filp, const char *buff, size_t len, loff_t *off){

	if(len > 20)
		len = 20;

	char *temp = (char*)kmalloc(sizeof(char)*20,GFP_KERNEL);
    
    strcpy(temp,bosluk);
    strcpy(msg,bosluk);
	
    memcpy(temp,buff,len);
    
	parser(temp);
    
    strcpy(temp,bosluk);
	kfree(temp);

	return len;
}

//proc un fonksiyonlarını tanımlama
static const struct file_operations hello_proc_fops = {
  .owner = THIS_MODULE,
  .read = my_proc_read,
  .write = my_proc_write,
};

//init modül fonksiyonu
static int __init hello_proc_init(void) {
  
  proc_create("hesapla", 0777, NULL, &hello_proc_fops);

  return 0;
}

//exit modül fonksiyonu
static void __exit hello_proc_exit(void) {
  remove_proc_entry("hesapla", NULL);

}

MODULE_LICENSE("GPL");
module_init(hello_proc_init);
module_exit(hello_proc_exit);
