#include "Engine/tkey.h"
#include "Engine/tkey_engine.h"
#include <string>
#include <cstdio>
#include <cstring>

static std::u16string screen;
static std::string toUtf8(const std::u16string &s) {
    std::string o;
    for (char16_t c : s) {
        if (c < 0x80) o += (char)c;
        else if (c < 0x800) { o += (char)(0xC0|(c>>6)); o += (char)(0x80|(c&0x3F)); }
        else { o += (char)(0xE0|(c>>12)); o += (char)(0x80|((c>>6)&0x3F)); o += (char)(0x80|(c&0x3F)); }
    }
    return o;
}
static void applyOutput(int r, unsigned int key) {
    if (!r) { if (key != 13) screen += (char16_t)key; return; }
    int bs = tkGetBackspaceLength();
    for (int i=0;i<bs && !screen.empty();i++) screen.pop_back();
    unsigned char buf[4104]; int len = tkGetBuff(buf);
    bool wide = !(tkGetOutType() == 1);
    if (wide) for (int i=0;i+1<len;i+=2) screen += (char16_t)(buf[i]|(buf[i+1]<<8));
    else for (int i=0;i<len;i++) screen += (char16_t)buf[i];
}
static void typeKeys(const char *s) {
    for (const char*p=s;*p;p++){
        if (*p=='<'){int r=tkPutBackspace();if(r)applyOutput(r,0);else if(!screen.empty())screen.pop_back();continue;}
        if (*p=='|'){tkResetBuff(0);continue;}
        applyOutput(tkProcessChar((unsigned char)*p),(unsigned char)*p);
    }
}
static std::string run(const char*k){tkResetBuff(0);screen.clear();typeKeys(k);return toUtf8(screen);}
static void setup() {
    tkSetup();
    tkEnableVietnamese(1); tkSetOutputCharset(0); tkSetEnableMacro(0);
    tkSetNewMarkStyle(1); tkSetFreeMarkStyle(1); tkSetEnableGrammarCheck(1);
    tkSetAutoRestore(1); tkSetAutoUpperCase(0); tkSetInputMethod(0);
}

struct T{const char*keys,*expect;};
static T tests[]={
 {"vieejt","việt"},{"tieengs","tiếng"},{"kieeur","kiểu"},{"kieeru","kiểu"},{"kieuer","kiểu"},
 {"kieeuf","kiều"},{"tieeuf","tiều"},{"yeeu","yêu"},{"yeeus","yếu"},{"nhieeuf","nhiều"},{"hieeur","hiểu"},
 {"chieeuf","chiều"},{"thieeus","thiếu"},{"dieeuj","diệu"},{"bieeur","biểu"},
 {"nguyeenx","nguyễn"},{"quyeenf","quyền"},{"khuyeen","khuyên"},{"truyeenf","truyền"},{"thuyeenf","thuyền"},
 {"dduowcj","được"},{"nguwowif","người"},{"nguowif","người"},{"muwowif","mười"},{"truwowngf","trường"},
 {"thuowngf","thường"},{"huwowngs","hướng"},{"ruwowuj","rượu"},{"huwu","hưu"},{"cuwuf","cừu"},
 {"hoaf","hoà"},{"khoer","khoẻ"},{"thuyr","thuỷ"},{"hoaij","hoại"},{"ngoaif","ngoài"},{"khuyur","khuỷu"},
 {"giaf","già"},{"gif","gì"},{"giuwax","giữa"},{"gieets","giết"},{"quas","quá"},{"quoocs","quốc"},{"quaan","quân"},
 {"ddeemf","đềm"},{"ddi","đi"},{"ddaau","đâu"},{"DDaau","Đâu"},{"Vieetj","Việt"},{"VIEETJ","VIỆT"},
 {"tooi","tôi"},{"baanf","bần"},{"chuyeenj","chuyện"},{"tuaanf","tuần"},{"luaatj","luật"},
 {"xuaans","xuấn"},{"ddoof","đồ"},{"cawts","cắt"},{"awn","ăn"},{"ow","ơ"},{"uw","ư"},
 {"oong","ông"},{"khoangr","khoảng"},{"hoawcj","hoặc"},{"loaij","loại"},
 {"tuwowngr","tưởng"},{"cuoois","cuối"},{"muoons","muốn"},{"thuoocj","thuộc"},{"nuwowcs","nước"},
 {"dduwowngf","đường"},{"khuyeens","khuyến"},{"oanh","oanh"},{"hoanhf","hoành"},{"uyeen","uyên"},
 {"gioongs","giống"},{"giuwowngf","giường"},{"quaf","quà"},{"queer","quể"},{"quyf","quỳ"},
 {"vietje","việt"},{"tiengse","tiếng"},{"nguoiwf","người"},{"dduocwj","được"},{"hoasn","hoán"},
 {"tieengsf","tiềng"},{"tieengsz","tiêng"},{"as","á"},{"ass","as"},{"aaa","aa"},{"ddd","dd"},
 {"Kieeur","Kiểu"},{"KIEEUR","KIỂU"},{"NGUWOWIF","NGƯỜI"},
 {"tieengs<","tiến"},{"kieeur<","kiể"},{"kieeur<<","ki"},
 {"windows ","windows "},{"class ","class "},{"text ","text "},{"tieengs ","tiếng "},{"kieeur ","kiểu "},
 {"tieengs vieejt","tiếng việt"},{"tooi yeeu Vieejt Nam","tôi yêu Việt Nam"},
 {"kieeur nhuw thees","kiểu như thế"},
 {"kieeu ","kiêu "},{"yeeu ","yêu "},{"kieeruz","kiêu"},
 {"nhw","như"},{"]","ư"},{"dd-m","đ-m"},
 {0,0}};

int main(){
  setup();
  int fail=0,n=0;
  for (T*t=tests;t->keys;t++){n++;
    std::string got=run(t->keys);
    if(got!=t->expect){fail++;printf("FAIL %-22s got '%s'  want '%s'\n",t->keys,got.c_str(),t->expect);}}
  printf("[table]       %d/%d passed\n",n-fail,n);

  setup();
  tkSetEnableMacro(1);
  tkAddKeyValueToMacroTable(tkGetCurrentMacroTable(), "qt", "quy\u1ebft t\u00e2m", 1);
  tkAddKeyValueToMacroTable(tkGetCurrentMacroTable(), "nn", "ng\u01b0\u1eddi nh\u1eadn", 1);
  {
    int mfail = 0;
    mfail += run("qt ") != "quy\u1ebft t\u00e2m ";
    mfail += run("nn ") != "ng\u01b0\u1eddi nh\u1eadn ";
    mfail += run("xy qt ") != "xy quy\u1ebft t\u00e2m ";
    mfail += run("qtj ") != "qtj ";
    printf("[macros]      %d/4 failed (want 0)\n", mfail);
    fail += mfail;
  }

  printf(fail?"\nFAILED\n":"\nALL PASSED\n");
  return fail?1:0;
}
