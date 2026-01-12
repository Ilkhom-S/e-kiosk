#include "keypud.h"
#include "ui_keypud.h"
#include "QTextCodec"
#include "qbuffer.h"

keyPud::keyPud(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::keyPud)
{
    ui->setupUi(this);

    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));


    shiftKEY = false;
    double_on = false;

     signalMapper.setMapping(ui->KEY_1,  ui->KEY_1);
     signalMapper.setMapping(ui->KEY_2,  ui->KEY_2);
     signalMapper.setMapping(ui->KEY_3,  ui->KEY_3);
     signalMapper.setMapping(ui->KEY_4,  ui->KEY_4);
     signalMapper.setMapping(ui->KEY_5,  ui->KEY_5);
     signalMapper.setMapping(ui->KEY_6,  ui->KEY_6);
     signalMapper.setMapping(ui->KEY_7,  ui->KEY_7);
     signalMapper.setMapping(ui->KEY_8,  ui->KEY_8);
     signalMapper.setMapping(ui->KEY_9,  ui->KEY_9);
     signalMapper.setMapping(ui->KEY_MINUS,  ui->KEY_MINUS);
     signalMapper.setMapping(ui->KEY_0,  ui->KEY_0);
     signalMapper.setMapping(ui->KEY_RAVNO,  ui->KEY_RAVNO);
     signalMapper.setMapping(ui->KEY_Q, ui->KEY_Q);
     signalMapper.setMapping(ui->KEY_W, ui->KEY_W);
     signalMapper.setMapping(ui->KEY_E, ui->KEY_E);
     signalMapper.setMapping(ui->KEY_R, ui->KEY_R);
     signalMapper.setMapping(ui->KEY_T, ui->KEY_T);
     signalMapper.setMapping(ui->KEY_Y, ui->KEY_Y);
     signalMapper.setMapping(ui->KEY_U, ui->KEY_U);
     signalMapper.setMapping(ui->KEY_I, ui->KEY_I);
     signalMapper.setMapping(ui->KEY_O, ui->KEY_O);
     signalMapper.setMapping(ui->KEY_P, ui->KEY_P);
     signalMapper.setMapping(ui->KEY_A, ui->KEY_A);
     signalMapper.setMapping(ui->KEY_S, ui->KEY_S);
     signalMapper.setMapping(ui->KEY_D, ui->KEY_D);
     signalMapper.setMapping(ui->KEY_F, ui->KEY_F);
     signalMapper.setMapping(ui->KEY_G, ui->KEY_G);
     signalMapper.setMapping(ui->KEY_H, ui->KEY_H);
     signalMapper.setMapping(ui->KEY_J, ui->KEY_J);
     signalMapper.setMapping(ui->KEY_K, ui->KEY_K);
     signalMapper.setMapping(ui->KEY_L, ui->KEY_L);
     signalMapper.setMapping(ui->KEY_Z, ui->KEY_Z);
     signalMapper.setMapping(ui->KEY_X, ui->KEY_X);
     signalMapper.setMapping(ui->KEY_C, ui->KEY_C);
     signalMapper.setMapping(ui->KEY_V, ui->KEY_V);
     signalMapper.setMapping(ui->KEY_B, ui->KEY_B);
     signalMapper.setMapping(ui->KEY_N, ui->KEY_N);
     signalMapper.setMapping(ui->KEY_M, ui->KEY_M);
     signalMapper.setMapping(ui->KEY_SEMICOLON, ui->KEY_SEMICOLON);
     signalMapper.setMapping(ui->KEY_COMMA, ui->KEY_COMMA);
     signalMapper.setMapping(ui->KEY_PERIOD, ui->KEY_PERIOD);
     signalMapper.setMapping(ui->KEY_SLASH, ui->KEY_SLASH);

     signalMapper.setMapping(ui->KEY_BACKSPACE, ui->KEY_BACKSPACE);
//     signalMapper.setMapping(ui->KEY_CLEAR, ui->KEY_CLEAR);
//     signalMapper.setMapping(ui->KEY_TAB, ui->KEY_TAB);
     signalMapper.setMapping(ui->KEY_ENTER, ui->KEY_ENTER);

     connect(ui->KEY_1, SIGNAL(released()),&signalMapper, SLOT(map()));
     connect(ui->KEY_2, SIGNAL(released()),&signalMapper, SLOT(map()));
     connect(ui->KEY_3, SIGNAL(released()),&signalMapper, SLOT(map()));
     connect(ui->KEY_4, SIGNAL(released()),&signalMapper, SLOT(map()));
     connect(ui->KEY_5, SIGNAL(released()),&signalMapper, SLOT(map()));
     connect(ui->KEY_6, SIGNAL(released()),&signalMapper, SLOT(map()));
     connect(ui->KEY_7, SIGNAL(released()),&signalMapper, SLOT(map()));
     connect(ui->KEY_8, SIGNAL(released()),&signalMapper, SLOT(map()));
     connect(ui->KEY_9, SIGNAL(released()), &signalMapper, SLOT(map()));
     connect(ui->KEY_MINUS, SIGNAL(released()),         &signalMapper, SLOT(map()));
     connect(ui->KEY_0, SIGNAL(released()),         &signalMapper, SLOT(map()));
     connect(ui->KEY_RAVNO, SIGNAL(released()),         &signalMapper, SLOT(map()));
     connect(ui->KEY_Q, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_W, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_E, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_R, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_T, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_Y, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_U, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_I, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_O, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_P, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_A, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_S, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_D, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_F, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_G, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_H, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_J, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_K, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_L, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_Z, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_X, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_C, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_V, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_B, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_N, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_M, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_SEMICOLON, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_COMMA, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_PERIOD, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_SLASH, SIGNAL(released()),     &signalMapper, SLOT(map()));
//     connect(ui->KEY_CLEAR, SIGNAL(released()),     &signalMapper, SLOT(map()));
     connect(ui->KEY_BACKSPACE, SIGNAL(released()), &signalMapper, SLOT(map()));
//     connect(ui->KEY_TAB, SIGNAL(released()), &signalMapper, SLOT(map()));
     connect(ui->KEY_ENTER, SIGNAL(released()),&signalMapper, SLOT(map()));

     //Нажат шифт
     connect(ui->KEY_SHIFT, SIGNAL(released()), this, SLOT(shiftClicked()));

     connect(&signalMapper, SIGNAL(mapped(QWidget *)), this, SLOT(buttonClicked(QWidget *)));

     ScharMap["KEY_1"] = '1';
     ScharMap["KEY_2"] = '2';
     ScharMap["KEY_3"] = '3';
     ScharMap["KEY_4"] = '4';
     ScharMap["KEY_5"] = '5';
     ScharMap["KEY_6"] = '6';
     ScharMap["KEY_7"] = '7';
     ScharMap["KEY_8"] = '8';
     ScharMap["KEY_9"] = '9';
     ScharMap["KEY_0"] = '0';
     ScharMap["KEY_MINUS"] = '-';
     ScharMap["KEY_RAVNO"] = '=';
     ScharMap["KEY_Q"] = 'q';
     ScharMap["KEY_W"] = 'w';
     ScharMap["KEY_E"] = 'e';
     ScharMap["KEY_R"] = 'r';
     ScharMap["KEY_T"] = 't';
     ScharMap["KEY_Y"] = 'y';
     ScharMap["KEY_U"] = 'u';
     ScharMap["KEY_I"] = 'i';
     ScharMap["KEY_O"] = 'o';
     ScharMap["KEY_P"] = 'p';
     ScharMap["KEY_A"] = 'a';
     ScharMap["KEY_S"] = 's';
     ScharMap["KEY_D"] = 'd';
     ScharMap["KEY_F"] = 'f';
     ScharMap["KEY_G"] = 'g';
     ScharMap["KEY_H"] = 'h';
     ScharMap["KEY_J"] = 'j';
     ScharMap["KEY_K"] = 'k';
     ScharMap["KEY_L"] = 'l';
     ScharMap["KEY_Z"] = 'z';
     ScharMap["KEY_X"] = 'x';
     ScharMap["KEY_C"] = 'c';
     ScharMap["KEY_V"] = 'v';
     ScharMap["KEY_B"] = 'b';
     ScharMap["KEY_N"] = 'n';
     ScharMap["KEY_M"] = 'm';
     ScharMap["KEY_SEMICOLON"] = ';';
     ScharMap["KEY_COMMA"] = ',';
     ScharMap["KEY_PERIOD"] = '.';
     ScharMap["KEY_SLASH"] = '/';
     ScharMap["KEY_CLEAR"] = '<<';
     ScharMap["KEY_BACKSPACE"] = '<-';
     ScharMap["KEY_TAB"] = '>>';
     ScharMap["KEY_ENTER"] = '>-';

      BcharMap["KEY_1"] = '!';
      BcharMap["KEY_2"] = '@';
      BcharMap["KEY_3"] = '#';
      BcharMap["KEY_4"] = '$';
      BcharMap["KEY_5"] = '%';
      BcharMap["KEY_6"] = '^';
      BcharMap["KEY_7"] = ' &';
      BcharMap["KEY_8"] = '*';
      BcharMap["KEY_9"] = '(';
      BcharMap["KEY_0"] = ')';
      BcharMap["KEY_MINUS"] = '_';
      BcharMap["KEY_RAVNO"] = '+';
      BcharMap["KEY_Q"] = 'Q';
      BcharMap["KEY_W"] = 'W';
      BcharMap["KEY_E"] = 'E';
      BcharMap["KEY_R"] = 'R';
      BcharMap["KEY_T"] = 'T';
      BcharMap["KEY_Y"] = 'Y';
      BcharMap["KEY_U"] = 'U';
      BcharMap["KEY_I"] = 'I';
      BcharMap["KEY_O"] = 'O';
      BcharMap["KEY_P"] = 'P';
      BcharMap["KEY_A"] = 'A';
      BcharMap["KEY_S"] = 'S';
      BcharMap["KEY_D"] = 'D';
      BcharMap["KEY_F"] = 'F';
      BcharMap["KEY_G"] = 'G';
      BcharMap["KEY_H"] = 'H';
      BcharMap["KEY_J"] = 'J';
      BcharMap["KEY_K"] = 'K';
      BcharMap["KEY_L"] = 'L';
      BcharMap["KEY_Z"] = 'Z';
      BcharMap["KEY_X"] = 'X';
      BcharMap["KEY_C"] = 'C';
      BcharMap["KEY_V"] = 'V';
      BcharMap["KEY_B"] = 'B';
      BcharMap["KEY_N"] = 'N';
      BcharMap["KEY_M"] = 'M';
      BcharMap["KEY_SEMICOLON"] = ':';
      BcharMap["KEY_COMMA"] = '<';
      BcharMap["KEY_PERIOD"] = '>';
      BcharMap["KEY_SLASH"] = '?';
      BcharMap["KEY_CLEAR"] = '<<';
      BcharMap["KEY_BACKSPACE"] = '<-';
      BcharMap["KEY_TAB"] = '>>';
      BcharMap["KEY_ENTER"] = '>-';

      DoublecharMap["KEY_1"] = '1';
      DoublecharMap["KEY_2"] = '2';
      DoublecharMap["KEY_3"] = '3';
      DoublecharMap["KEY_4"] = '4';
      DoublecharMap["KEY_5"] = '5';
      DoublecharMap["KEY_6"] = '6';
      DoublecharMap["KEY_7"] = '7';
      DoublecharMap["KEY_8"] = '8';
      DoublecharMap["KEY_9"] = '9';
      DoublecharMap["KEY_0"] = '0';
      DoublecharMap["KEY_MINUS"] = '_';
      DoublecharMap["KEY_RAVNO"] = '+';
      DoublecharMap["KEY_Q"] = 'Q';
      DoublecharMap["KEY_W"] = 'W';
      DoublecharMap["KEY_E"] = 'E';
      DoublecharMap["KEY_R"] = 'R';
      DoublecharMap["KEY_T"] = 'T';
      DoublecharMap["KEY_Y"] = 'Y';
      DoublecharMap["KEY_U"] = 'U';
      DoublecharMap["KEY_I"] = 'I';
      DoublecharMap["KEY_O"] = 'O';
      DoublecharMap["KEY_P"] = 'P';
      DoublecharMap["KEY_A"] = 'A';
      DoublecharMap["KEY_S"] = 'S';
      DoublecharMap["KEY_D"] = 'D';
      DoublecharMap["KEY_F"] = 'F';
      DoublecharMap["KEY_G"] = 'G';
      DoublecharMap["KEY_H"] = 'H';
      DoublecharMap["KEY_J"] = 'J';
      DoublecharMap["KEY_K"] = 'K';
      DoublecharMap["KEY_L"] = 'L';
      DoublecharMap["KEY_Z"] = 'Z';
      DoublecharMap["KEY_X"] = 'X';
      DoublecharMap["KEY_C"] = 'C';
      DoublecharMap["KEY_V"] = 'V';
      DoublecharMap["KEY_B"] = 'B';
      DoublecharMap["KEY_N"] = 'N';
      DoublecharMap["KEY_M"] = 'M';
      DoublecharMap["KEY_SEMICOLON"] = ':';
      DoublecharMap["KEY_COMMA"] = '<';
      DoublecharMap["KEY_PERIOD"] = '>';
      DoublecharMap["KEY_SLASH"] = '?';
      DoublecharMap["KEY_CLEAR"] = "<<";
      DoublecharMap["KEY_BACKSPACE"] = "<-";
      DoublecharMap["KEY_TAB"] = ">>";
      DoublecharMap["KEY_ENTER"] = ">-";

      //Кирилица
      DoublecharMap["RU_KEY_1"] = '1';
      DoublecharMap["RU_KEY_2"] = '2';
      DoublecharMap["RU_KEY_3"] = '3';
      DoublecharMap["RU_KEY_4"] = '4';
      DoublecharMap["RU_KEY_5"] = '5';
      DoublecharMap["RU_KEY_6"] = '6';
      DoublecharMap["RU_KEY_7"] = '7';
      DoublecharMap["RU_KEY_8"] = '8';
      DoublecharMap["RU_KEY_9"] = '9';
      DoublecharMap["RU_KEY_0"] = '0';
      DoublecharMap["RU_KEY_MINUS"] = tr("Х");
      DoublecharMap["RU_KEY_RAVNO"] = tr("Ъ");
      DoublecharMap["RU_KEY_Q"] = tr("Й");
      DoublecharMap["RU_KEY_W"] = tr("Ц");
      DoublecharMap["RU_KEY_E"] = tr("У");
      DoublecharMap["RU_KEY_R"] = tr("К");
      DoublecharMap["RU_KEY_T"] = tr("Е");
      DoublecharMap["RU_KEY_Y"] = tr("Н");
      DoublecharMap["RU_KEY_U"] = tr("Г");
      DoublecharMap["RU_KEY_I"] = tr("Ш");
      DoublecharMap["RU_KEY_O"] = tr("Щ");
      DoublecharMap["RU_KEY_P"] = tr("З");
      DoublecharMap["RU_KEY_A"] = tr("Ф");
      DoublecharMap["RU_KEY_S"] = tr("Ы");
      DoublecharMap["RU_KEY_D"] = tr("В");
      DoublecharMap["RU_KEY_F"] = tr("А");
      DoublecharMap["RU_KEY_G"] = tr("П");
      DoublecharMap["RU_KEY_H"] = tr("Р");
      DoublecharMap["RU_KEY_J"] = tr("О");
      DoublecharMap["RU_KEY_K"] = tr("Л");
      DoublecharMap["RU_KEY_L"] = tr("Д");
      DoublecharMap["RU_KEY_Z"] = tr("Я");
      DoublecharMap["RU_KEY_X"] = tr("Ч");
      DoublecharMap["RU_KEY_C"] = tr("С");
      DoublecharMap["RU_KEY_V"] = tr("М");
      DoublecharMap["RU_KEY_B"] = tr("И");
      DoublecharMap["RU_KEY_N"] = tr("Т");
      DoublecharMap["RU_KEY_M"] = tr("Ь");
      DoublecharMap["RU_KEY_SEMICOLON"] = tr("Ж");
      DoublecharMap["RU_KEY_COMMA"] = tr("Б");
      DoublecharMap["RU_KEY_PERIOD"] = tr("Ю");
      DoublecharMap["RU_KEY_SLASH"] =  tr("Э");
      DoublecharMap["RU_KEY_CLEAR"] = "<<";
      DoublecharMap["RU_KEY_BACKSPACE"] = "<-";
      DoublecharMap["RU_KEY_TAB"] = ">>";
      DoublecharMap["RU_KEY_ENTER"] = ">-";

}

void keyPud::changeToRU(){
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("cp1251"));
}
void keyPud::changeToEN(){
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
}

void keyPud::fordouble(QString lang)
{
    gblLang = lang;
    double_on = true;

    if(lang == "EN"){
        ui->KEY_1->setText(DoublecharMap["KEY_1"]);
        ui->KEY_2->setText(DoublecharMap["KEY_2"]);
        ui->KEY_3->setText(DoublecharMap["KEY_3"]);
        ui->KEY_4->setText(DoublecharMap["KEY_4"]);
        ui->KEY_5->setText(DoublecharMap["KEY_5"]);
        ui->KEY_6->setText(DoublecharMap["KEY_6"]);
        ui->KEY_7->setText(DoublecharMap["KEY_7"]);
        ui->KEY_8->setText(DoublecharMap["KEY_8"]);
        ui->KEY_9->setText(DoublecharMap["KEY_9"]);
        ui->KEY_0->setText(DoublecharMap["KEY_0"]);
        ui->KEY_MINUS->setText(DoublecharMap["KEY_MINUS"]);
        ui->KEY_RAVNO->setText(DoublecharMap["KEY_RAVNO"]);
        ui->KEY_Q->setText(DoublecharMap["KEY_Q"]);
        ui->KEY_W->setText(DoublecharMap["KEY_W"]);
        ui->KEY_E->setText(DoublecharMap["KEY_E"]);
        ui->KEY_R->setText(DoublecharMap["KEY_R"]);
        ui->KEY_T->setText(DoublecharMap["KEY_T"]);
        ui->KEY_Y->setText(DoublecharMap["KEY_Y"]);
        ui->KEY_U->setText(DoublecharMap["KEY_U"]);
        ui->KEY_I->setText(DoublecharMap["KEY_I"]);
        ui->KEY_O->setText(DoublecharMap["KEY_O"]);
        ui->KEY_P->setText(DoublecharMap["KEY_P"]);
        ui->KEY_A->setText(DoublecharMap["KEY_A"]);
        ui->KEY_S->setText(DoublecharMap["KEY_S"]);
        ui->KEY_D->setText(DoublecharMap["KEY_D"]);
        ui->KEY_F->setText(DoublecharMap["KEY_F"]);
        ui->KEY_G->setText(DoublecharMap["KEY_G"]);
        ui->KEY_H->setText(DoublecharMap["KEY_H"]);
        ui->KEY_J->setText(DoublecharMap["KEY_J"]);
        ui->KEY_K->setText(DoublecharMap["KEY_K"]);
        ui->KEY_L->setText(DoublecharMap["KEY_L"]);
        ui->KEY_Z->setText(DoublecharMap["KEY_Z"]);
        ui->KEY_X->setText(DoublecharMap["KEY_X"]);
        ui->KEY_C->setText(DoublecharMap["KEY_C"]);
        ui->KEY_V->setText(DoublecharMap["KEY_V"]);
        ui->KEY_B->setText(DoublecharMap["KEY_B"]);
        ui->KEY_N->setText(DoublecharMap["KEY_N"]);
        ui->KEY_M->setText(DoublecharMap["KEY_M"]);

        ui->KEY_SEMICOLON->setText(DoublecharMap["KEY_SEMICOLON"]);
        ui->KEY_COMMA->setText(DoublecharMap["KEY_COMMA"]);
        ui->KEY_PERIOD->setText(DoublecharMap["KEY_PERIOD"]);
        ui->KEY_SLASH->setText(DoublecharMap["KEY_SLASH"]);

    }else if(lang == "RU"){



        ui->KEY_1->setText(DoublecharMap["RU_KEY_1"]);
        ui->KEY_2->setText(DoublecharMap["RU_KEY_2"]);
        ui->KEY_3->setText(DoublecharMap["RU_KEY_3"]);
        ui->KEY_4->setText(DoublecharMap["RU_KEY_4"]);
        ui->KEY_5->setText(DoublecharMap["RU_KEY_5"]);
        ui->KEY_6->setText(DoublecharMap["RU_KEY_6"]);
        ui->KEY_7->setText(DoublecharMap["RU_KEY_7"]);
        ui->KEY_8->setText(DoublecharMap["RU_KEY_8"]);
        ui->KEY_9->setText(DoublecharMap["RU_KEY_9"]);
        ui->KEY_0->setText(DoublecharMap["RU_KEY_0"]);
        ui->KEY_MINUS->setText(DoublecharMap["RU_KEY_MINUS"]);
        ui->KEY_RAVNO->setText(DoublecharMap["RU_KEY_RAVNO"]);
        ui->KEY_Q->setText(DoublecharMap["RU_KEY_Q"]);
        ui->KEY_W->setText(DoublecharMap["RU_KEY_W"]);
        ui->KEY_E->setText(DoublecharMap["RU_KEY_E"]);
        ui->KEY_R->setText(DoublecharMap["RU_KEY_R"]);
        ui->KEY_T->setText(DoublecharMap["RU_KEY_T"]);
        ui->KEY_Y->setText(DoublecharMap["RU_KEY_Y"]);
        ui->KEY_U->setText(DoublecharMap["RU_KEY_U"]);
        ui->KEY_I->setText(DoublecharMap["RU_KEY_I"]);
        ui->KEY_O->setText(DoublecharMap["RU_KEY_O"]);
        ui->KEY_P->setText(DoublecharMap["RU_KEY_P"]);
        ui->KEY_A->setText(DoublecharMap["RU_KEY_A"]);
        ui->KEY_S->setText(DoublecharMap["RU_KEY_S"]);
        ui->KEY_D->setText(DoublecharMap["RU_KEY_D"]);
        ui->KEY_F->setText(DoublecharMap["RU_KEY_F"]);
        ui->KEY_G->setText(DoublecharMap["RU_KEY_G"]);
        ui->KEY_H->setText(DoublecharMap["RU_KEY_H"]);
        ui->KEY_J->setText(DoublecharMap["RU_KEY_J"]);
        ui->KEY_K->setText(DoublecharMap["RU_KEY_K"]);
        ui->KEY_L->setText(DoublecharMap["RU_KEY_L"]);
        ui->KEY_Z->setText(DoublecharMap["RU_KEY_Z"]);
        ui->KEY_X->setText(DoublecharMap["RU_KEY_X"]);
        ui->KEY_C->setText(DoublecharMap["RU_KEY_C"]);
        ui->KEY_V->setText(DoublecharMap["RU_KEY_V"]);
        ui->KEY_B->setText(DoublecharMap["RU_KEY_B"]);
        ui->KEY_N->setText(DoublecharMap["RU_KEY_N"]);
        ui->KEY_M->setText(DoublecharMap["RU_KEY_M"]);

        ui->KEY_SEMICOLON->setText(DoublecharMap["RU_KEY_SEMICOLON"]);
        ui->KEY_COMMA->setText(DoublecharMap["RU_KEY_COMMA"]);
        ui->KEY_PERIOD->setText(DoublecharMap["RU_KEY_PERIOD"]);
        ui->KEY_SLASH->setText(DoublecharMap["RU_KEY_SLASH"]);
    }



}

void keyPud::shiftClicked()
{

    //для ГАИ? отключаем шифт
    if(double_on)
        return;

       if (!shiftKEY){
           ui->KEY_1->setText(BcharMap["KEY_1"]);
           ui->KEY_2->setText(BcharMap["KEY_2"]);
           ui->KEY_3->setText(BcharMap["KEY_3"]);
           ui->KEY_4->setText(BcharMap["KEY_4"]);
           ui->KEY_5->setText(BcharMap["KEY_5"]);
           ui->KEY_6->setText(BcharMap["KEY_6"]);
           ui->KEY_7->setText(BcharMap["KEY_7"]);
           ui->KEY_8->setText(BcharMap["KEY_8"]);
           ui->KEY_9->setText(BcharMap["KEY_9"]);
           ui->KEY_0->setText(BcharMap["KEY_0"]);
           ui->KEY_MINUS->setText(BcharMap["KEY_MINUS"]);
           ui->KEY_RAVNO->setText(BcharMap["KEY_RAVNO"]);
           ui->KEY_Q->setText(BcharMap["KEY_Q"]);
           ui->KEY_W->setText(BcharMap["KEY_W"]);
           ui->KEY_E->setText(BcharMap["KEY_E"]);
           ui->KEY_R->setText(BcharMap["KEY_R"]);
           ui->KEY_T->setText(BcharMap["KEY_T"]);
           ui->KEY_Y->setText(BcharMap["KEY_Y"]);
           ui->KEY_U->setText(BcharMap["KEY_U"]);
           ui->KEY_I->setText(BcharMap["KEY_I"]);
           ui->KEY_O->setText(BcharMap["KEY_O"]);
           ui->KEY_P->setText(BcharMap["KEY_P"]);
           ui->KEY_A->setText(BcharMap["KEY_A"]);
           ui->KEY_S->setText(BcharMap["KEY_S"]);
           ui->KEY_D->setText(BcharMap["KEY_D"]);
           ui->KEY_F->setText(BcharMap["KEY_F"]);
           ui->KEY_G->setText(BcharMap["KEY_G"]);
           ui->KEY_H->setText(BcharMap["KEY_H"]);
           ui->KEY_J->setText(BcharMap["KEY_J"]);
           ui->KEY_K->setText(BcharMap["KEY_K"]);
           ui->KEY_L->setText(BcharMap["KEY_L"]);
           ui->KEY_Z->setText(BcharMap["KEY_Z"]);
           ui->KEY_X->setText(BcharMap["KEY_X"]);
           ui->KEY_C->setText(BcharMap["KEY_C"]);
           ui->KEY_V->setText(BcharMap["KEY_V"]);
           ui->KEY_B->setText(BcharMap["KEY_B"]);
           ui->KEY_N->setText(BcharMap["KEY_N"]);
           ui->KEY_M->setText(BcharMap["KEY_M"]);

           ui->KEY_SEMICOLON->setText(BcharMap["KEY_SEMICOLON"]);
           ui->KEY_COMMA->setText(BcharMap["KEY_COMMA"]);
           ui->KEY_PERIOD->setText(BcharMap["KEY_PERIOD"]);
           ui->KEY_SLASH->setText(BcharMap["KEY_SLASH"]);

       }else{

           ui->KEY_1->setText(ScharMap["KEY_1"]);
           ui->KEY_2->setText(ScharMap["KEY_2"]);
           ui->KEY_3->setText(ScharMap["KEY_3"]);
           ui->KEY_4->setText(ScharMap["KEY_4"]);
           ui->KEY_5->setText(ScharMap["KEY_5"]);
           ui->KEY_6->setText(ScharMap["KEY_6"]);
           ui->KEY_7->setText(ScharMap["KEY_7"]);
           ui->KEY_8->setText(ScharMap["KEY_8"]);
           ui->KEY_9->setText(ScharMap["KEY_9"]);
           ui->KEY_0->setText(ScharMap["KEY_0"]);
           ui->KEY_MINUS->setText(ScharMap["KEY_MINUS"]);
           ui->KEY_RAVNO->setText(ScharMap["KEY_RAVNO"]);
           ui->KEY_Q->setText(ScharMap["KEY_Q"]);
           ui->KEY_W->setText(ScharMap["KEY_W"]);
           ui->KEY_E->setText(ScharMap["KEY_E"]);
           ui->KEY_R->setText(ScharMap["KEY_R"]);
           ui->KEY_T->setText(ScharMap["KEY_T"]);
           ui->KEY_Y->setText(ScharMap["KEY_Y"]);
           ui->KEY_U->setText(ScharMap["KEY_U"]);
           ui->KEY_I->setText(ScharMap["KEY_I"]);
           ui->KEY_O->setText(ScharMap["KEY_O"]);
           ui->KEY_P->setText(ScharMap["KEY_P"]);
           ui->KEY_A->setText(ScharMap["KEY_A"]);
           ui->KEY_S->setText(ScharMap["KEY_S"]);
           ui->KEY_D->setText(ScharMap["KEY_D"]);
           ui->KEY_F->setText(ScharMap["KEY_F"]);
           ui->KEY_G->setText(ScharMap["KEY_G"]);
           ui->KEY_H->setText(ScharMap["KEY_H"]);
           ui->KEY_J->setText(ScharMap["KEY_J"]);
           ui->KEY_K->setText(ScharMap["KEY_K"]);
           ui->KEY_L->setText(ScharMap["KEY_L"]);
           ui->KEY_Z->setText(ScharMap["KEY_Z"]);
           ui->KEY_X->setText(ScharMap["KEY_X"]);
           ui->KEY_C->setText(ScharMap["KEY_C"]);
           ui->KEY_V->setText(ScharMap["KEY_V"]);
           ui->KEY_B->setText(ScharMap["KEY_B"]);
           ui->KEY_N->setText(ScharMap["KEY_N"]);
           ui->KEY_M->setText(ScharMap["KEY_M"]);

           ui->KEY_SEMICOLON->setText(ScharMap["KEY_SEMICOLON"]);
           ui->KEY_COMMA->setText(ScharMap["KEY_COMMA"]);
           ui->KEY_PERIOD->setText(ScharMap["KEY_PERIOD"]);
           ui->KEY_SLASH->setText(ScharMap["KEY_SLASH"]);

       }

       if(shiftKEY) shiftKEY = false;
       else shiftKEY = true;
}

void keyPud::buttonClicked(QWidget *w)
{

    QString on = w->objectName();
    QChar chr;

    QString RU_change = "";

    if(gblLang == "RU")
        RU_change = "RU_";

       if (shiftKEY){
            chr = BcharMap[on];
            shiftClicked();
        }else if(double_on)
        {
             if((on == "KEY_CLEAR") || (on == "KEY_BACKSPACE") || (on == "KEY_TAB") || (on == "KEY_ENTER"))
                chr = BcharMap[on];
            else{
                QString strf = DoublecharMap[RU_change + on];
                chr = strf.at(0);
            }

            fordouble(gblLang);
        }
        else{
            chr = ScharMap[on];
       }

//    qDebug() << "w->objectName() - " << on;
//    qDebug() << "chr - " << chr;

    emit characterGenerated(chr);
}

keyPud::~keyPud()
{
    delete ui;
}


void keyPud::clickBackspace()
{
    ui->KEY_BACKSPACE->click();
}
