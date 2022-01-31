//Bibliotecas//
#include <Wire.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include "RTClib.h"

//Definições//
#define EEPROM_MAX_SIZE 512

//Declarações de Funções//
void escrever_hora(String hora);
void escrever_horario(String hora, int posicao_ini);
void escrever_nome(String nome, int posicao_ini);
String ler_horario(int posicao_ini);
String ler_nome(int posicao_ini);
void mudar_lampada(int lamp, int val);
void avancar_con_anc();
void html_index();
void html_config();

//Variáveis Globais//
boolean rele[] = {0, 0, 0, 0, 0, 0, 0, 0};
byte pino[] = {18, 19, 13, 27, 26, 25, 33, 32};
int pi_palavras[] = {288, 309, 330, 351, 372, 393, 414, 435};
int pi_con = 0;
int pi_anc = 456;
byte att = 0;
unsigned long tempo = millis();

//Objetos//
RTC_DS3231 rtc;
WebServer server(80);

void setup()
{
    //Iniciar Objetos//
    rtc.begin();
    EEPROM.begin(EEPROM_MAX_SIZE);

    //Modo dos pinos//
    pinMode(13, OUTPUT);
    pinMode(18, OUTPUT);
    pinMode(19, OUTPUT);
    pinMode(25, OUTPUT);
    pinMode(26, OUTPUT);
    pinMode(27, OUTPUT);
    pinMode(32, OUTPUT);
    pinMode(33, OUTPUT);

    //Checagem inicial//
    int pa = 456;
    while (pa != 488)
    {
        pi_con = EEPROM.read(pa);
        if (pi_con != 0xff)
        {
            pi_anc = pa;
            break;
        }
        pa++;
    }
    for (byte i = 0; i < 8; i++)
    {
        rele[i] = EEPROM.read(pi_con + i);
        digitalWrite(pino[i], rele[i]);
    }

    //Nome de rede e senha//
    const char *ssid = "Rede de lâmpadas";
    const char *password = "teste123";

    //Configurações de Rede//
    IPAddress staticIP(222, 111, 2, 1);
    IPAddress gateway(222, 111, 2, 0);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(staticIP, gateway, subnet);

    //Páginas web e rotas, iniciar objeto server//
    server.on("/", html_index);
    server.on("/config", html_config);
    server.begin();
}

void loop()
{
    //Variáveis//
    server.handleClient();
    DateTime now = rtc.now();
    byte hora = now.hour();
    byte minu = now.minute();
    int i = 256, j = 0;
    unsigned long current = millis();

    //Código principal de ativação e desativação//
    if ((current - tempo) >= 1000)
    {
        tempo = current;
        for (i = 256; i < 285; i += 4)
        {
            if (EEPROM.read(i) == hora && EEPROM.read(i + 1) == minu)
            {
                digitalWrite(pino[j], 1);
                rele[j] = 1;
                avancar_con_anc();
            }
            if (EEPROM.read(i + 2) == hora && EEPROM.read(i + 3) == minu)
            {
                digitalWrite(pino[j], 0);
                rele[j] = 0;
                avancar_con_anc();
            }
            j++;
        }
    }
}

void escrever_hora(String hora)
{
    DateTime now = rtc.now();
    String h = "", m = "";
    for (byte i = 0; i < 2; i++)
    {
        h += hora[i];
        m += hora[i + 3];
    }
    int h1 = h.toInt();
    int m1 = m.toInt();
    rtc.adjust(DateTime(now.year(), now.month(), now.day(), h1, m1, 0));
}

void escrever_horario(String hora, int posicao_ini)
{
    String h, m;
    for (byte i = 0; i < 2; i++)
    {
        h += hora[i];
        m += hora[i + 3];
    }
    EEPROM.write(posicao_ini, h.toInt());
    EEPROM.write(posicao_ini + 1, m.toInt());
    EEPROM.commit();
}

void escrever_nome(String nome, int posicao_ini)
{
    char tam = nome.length();
    char i;
    for (i = 0; i < tam; i++)
    {
        EEPROM.write(posicao_ini + i, nome[i]);
    }
    EEPROM.write(posicao_ini + tam, '\0');
    EEPROM.commit();
}

String ler_horario(int posicao_ini)
{
    String hora;
    if (EEPROM.read(posicao_ini) < 10)
    {
        hora = "0" + String(EEPROM.read(posicao_ini));
    }
    else
    {
        hora = String(EEPROM.read(posicao_ini));
    }
    hora += ":";
    if (EEPROM.read(posicao_ini + 1) < 10)
    {
        hora += "0" + String(EEPROM.read(posicao_ini + 1));
    }
    else
    {
        hora += String(EEPROM.read(posicao_ini + 1));
    }
    return hora;
}

String ler_nome(int posicao_ini)
{
    char nome[21];
    unsigned char c;
    char i = 0;
    while (c != '\0' && i < 21)
    {
        c = EEPROM.read(posicao_ini + i);
        nome[i] = c;
        i++;
    }
    nome[i] = '\0';
    return String(nome);
}

void mudar_lampada(int lamp, int val)
{
    rele[lamp - 1] = val;
    digitalWrite(pino[lamp - 1], val);
    avancar_con_anc();
}

void avancar_con_anc()
{
    if (pi_con + 8 > 248)
    {
        pi_con = 0;
    }
    else
    {
        pi_con += 8;
    }

    if (pi_anc < 487)
    {
        EEPROM.write(pi_anc + 1, pi_con);
        EEPROM.write(pi_anc, 0xff);
        pi_anc += 1;
    }
    else
    {
        pi_anc = 456;
        EEPROM.write(pi_anc, pi_con);
        EEPROM.write(487, 0xff);
    }

    for (byte c = 0; c < 8; c++)
    {
        EEPROM.write(pi_con + c, rele[c]);
    }
    EEPROM.commit();
}

void html_index()
{
    DateTime now = rtc.now();
    boolean p = 0;
    if (server.hasArg("lampada"))
    {
        if (server.hasArg("valor"))
        {
            mudar_lampada(server.arg("lampada").toInt(), server.arg("valor").toInt());
        }
        else
        {
            mudar_lampada(server.arg("lampada").toInt(), 0);
        }
        p = 1;
    }
    String pagina = "\
        <!DOCTYPE html>\
        <html lang='pt-br'>\
        <head>\
            <title>Controle de lâmpadas</title>\
            <meta charset='utf-8'>\
            <meta name='viewport' content='width=device-width, initial-scale=1'>\
            <script>\
                function processo(formulario, entrada){\
                    input_mudar = document.getElementById(entrada);\
                    input_mudar.value = input_mudar.checked ? '1':'0';\
                    document.getElementById(formulario).submit();\
                }\
            </script>\
            <style>\
                * {\
                    padding: 0;\
                    margin: 0;\
                    text-decoration: none;\
                    font-family: Trebuchet MS, sans-serif;\
                }\
                main {\
                    display: flex;\
                    flex-direction: column;\
                    align-items: center;\
                }\
                h1 {\
                    text-decoration: underline;\
                    margin-top: 4%;\
                    margin-bottom: 2%;\
                    font-family: Trebuchet MS, sans-serif;\
                    font-size: 28px;\
                    height: 5vh;\
                }\
                h2 {\
                    margin-left: 5%;\
                    font-size: 12px;\
                    font-family: Trebuchet MS, sans-serif;\
                    color: white;\
                }\
                h3 {\
                    text-decoration: underline;\
                    font-size: 10px;\
                    font-family: Trebuchet MS, sans-serif;\
                    margin-left: 10%;\
                    margin-right: 5%;\
                    color: white;\
                }\
                h4 {\
                    background-color: rgb(255, 203, 203);\
                    border-radius: 10px;\
                    padding: 5px;\
                    font-size: 20px;\
                    font-weight: 400;\
                    margin-left: 2%;\
                }\
                section {\
                    display: flex;\
                    flex-direction: column;\
                    border-radius: 30px;\
                    background-color: rgb(240, 240, 240);\
                    box-shadow: 0px 0px 7px black;\
                    width: 94vw;\
                    height: 70vh;\
                }\
                a{\
                    background-color: rgb(255, 203, 203);\
                    border-radius: 10px;\
                    padding: 5px;\
                    color: black;\
                    font-size: 20px;\
                    margin-right: 2%;\
                }\
                .linha {\
                    display: flex;\
                    flex-direction: row;\
                    align-items: center;\
                    width: 100%;\
                    height: 22.5%;\
                }\
                .linha_div {\
                    display: flex;\
                    flex-direction: row;\
                    border-radius: 20px;\
                    box-shadow: 3px 3px 5px rgb(60, 60, 60);\
                    margin-right: 2%;\
                    margin-left: 2%;\
                    width: 50%;\
                    height: 50%;\
                }\
                .linha_div1 {\
                    display: flex;\
                    align-items: center;\
                    width: 58%;\
                }\
                .linha_div2 {\
                    display: flex;\
                    align-items: center;\
                    width: 17%;\
                }\
                .linha_div3 {\
                    display: flex;\
                    align-items: center;\
                    width: 13%;\
                }\
                .hora_config{\
                    display: flex;\
                    flex-direction: row;\
                    justify-content: space-between;\
                }\
                input[type='checkbox'] {\
                    margin-left: 10px !important;\
                    margin-bottom: -5px !important;\
                    position: relative;\
                    width: 30px;\
                    height: 16px;\
                    -webkit-appearance: none;\
                    background: #B3B3B3;\
                    outline: none;\
                    border-radius: 15px;\
                    box-shadow: inset 0 0 5px rgba(0, 0, 0, .2);\
                    transition: .5s;\
                }\
                input:checked[type='checkbox'] {\
                    background: rgb(120, 233, 105);\
                }\
                input[type='checkbox']:before {\
                    content: '';\
                    position: absolute;\
                    width: 15px;\
                    height: 15px;\
                    border-radius: 15px;\
                    top: 0;\
                    left: 0;\
                    background: #ffffff;\
                    transform: scale(1.1);\
                    box-shadow: 0 2px 5px rgba(0, 0, 0, .2);\
                    transition: .5s;\
                }\
                input:checked[type='checkbox']:before {\
                    left: 16px;\
                }\
                p{\
                    margin: 20px;\
                    flex-wrap: wrap;\
                }\
                @media screen and (min-width: 1000px){\
                    section{\
                        width: 70vw;\
                    }\
                    h2{\
                        font-size: 20px;\
                    }\
                    h3{\
                        font-size: 14px;\
                    }\
                    input[type='checkbox']{\
                        width: 40px;\
                        height: 18px;\
                    }\
                    input[type='checkbox']:before{\
                        width: 18px;\
                        height: 18px;\
                    }\
                    input:checked[type='checkbox']:before{\
                        left: 22px;\
                    }\
                    form{\
                        margin-left: 50%;\
                    }\
                }\
            </style>\
        </head>\
        <body>\
            <main>\
                <h1>Controle de Lâmpadas</h1>\
                <section>\
                    <div class='linha'>\
                        <div class='linha_div' id='fundo1'>\
                            <div class='linha_div1'>\
                                <h2>";
                                    pagina += ler_nome(288);
                                    pagina += "\
                                </h2>\
                            </div>\
                            <div class='linha_div2'>\
                                <h3>";
                                    pagina += ler_horario(256);
                                    pagina += "-";
                                    pagina += ler_horario(258);
                                    pagina += "\
                                </h3>\
                            </div>\
                            <div class='linha_div3'>\
                                <form id='form_1' method='POST'>\
                                    <input type='checkbox' name='valor' id='id_l1' onclick=\"processo(";
                                        pagina += "\'form_1\',";
                                        pagina += "\'id_l1\')";
                                        pagina += "\">";
                                        pagina += "\
                                    <input type='hidden' name='lampada' value='1'>\
                                </form>\
                            </div>\
                        </div>\
                        <div class='linha_div' id='fundo2'>\
                            <div class='linha_div1'>\
                                <h2>";
                                    pagina += ler_nome(309);
                                    pagina += "\
                                </h2>\
                            </div>\
                            <div class='linha_div2'>\
                                <h3>";
                                    pagina += ler_horario(260);
                                    pagina += "-";
                                    pagina += ler_horario(262);
                                    pagina += "\
                                </h3>\
                            </div>\
                            <div class='linha_div3'>\
                                <form id='form_2' method='POST'>\
                                    <input type='checkbox' name='valor' id='id_l2' onclick=\"processo(";
                                        pagina += "\'form_2\',";
                                        pagina += "\'id_l2\')";
                                        pagina += "\">";
                                        pagina += "\
                                    <input type='hidden' name='lampada' value='2'>\
                                </form>\
                            </div>\
                        </div>\
                    </div>\
                    <div class='linha'>\
                        <div class='linha_div' id='fundo3'>\
                            <div class='linha_div1'>\
                                <h2>";
                                    pagina += ler_nome(330);
                                    pagina += "\
                                </h2>\
                            </div>\
                            <div class='linha_div2'>\
                                <h3>";
                                    pagina += ler_horario(264);
                                    pagina += "-";
                                    pagina += ler_horario(266);
                                    pagina += "\
                                </h3>\
                            </div>\
                            <div class='linha_div3'>\
                                <form id='form_3' method='POST'>\
                                    <input type='checkbox' name='valor' id='id_l3' onclick=\"processo(";
                                        pagina += "\'form_3\',";
                                        pagina += "\'id_l3\')";
                                        pagina += "\">";
                                        pagina += "\
                                    <input type='hidden' name='lampada' value='3'>\
                                </form>\
                            </div>\
                        </div>\
                        <div class='linha_div' id='fundo4'>\
                            <div class='linha_div1'>\
                                <h2>";
                                    pagina += ler_nome(351);
                                    pagina += "\
                                </h2>\
                            </div>\
                            <div class='linha_div2'>\
                                <h3>";
                                    pagina += ler_horario(268);
                                    pagina += "-";
                                    pagina += ler_horario(270);
                                    pagina += "\
                                </h3>\
                            </div>\
                            <div class='linha_div3'>\
                                <form id='form_4' method='POST'>\
                                    <input type='checkbox' name='valor' id='id_l4' onclick=\"processo(";
                                        pagina += "\'form_4\',";
                                        pagina += "\'id_l4\')";
                                        pagina += "\">";
                                        pagina += "\
                                    <input type='hidden' name='lampada' value='4'>\
                                </form>\
                            </div>\
                        </div>\
                    </div>\
                    <div class='linha'>\
                        <div class='linha_div' id='fundo5'>\
                            <div class='linha_div1'>\
                                <h2>";
                                    pagina += ler_nome(372);
                                    pagina += "\
                                </h2>\
                            </div>\
                            <div class='linha_div2'>\
                                <h3>";
                                    pagina += ler_horario(272);
                                    pagina += "-";
                                    pagina += ler_horario(274);
                                    pagina += "\
                                </h3>\
                            </div>\
                            <div class='linha_div3'>\
                                <form id='form_5' method='POST'>\
                                    <input type='checkbox' name='valor' id='id_l5' onclick=\"processo(";
                                        pagina += "\'form_5\',";
                                        pagina += "\'id_l5\')";
                                        pagina += "\">";
                                        pagina += "\
                                    <input type='hidden' name='lampada' value='5'>\
                                </form>\
                            </div>\
                        </div>\
                        <div class='linha_div' id='fundo6'>\
                            <div class='linha_div1'>\
                                <h2>";
                                    pagina += ler_nome(393);
                                    pagina += "\
                                </h2>\
                            </div>\
                            <div class='linha_div2'>\
                                <h3>";
                                    pagina += ler_horario(276);
                                    pagina += "-";
                                    pagina += ler_horario(278);
                                    pagina += "\
                                </h3>\
                            </div>\
                            <div class='linha_div3'>\
                                <form id='form_6' method='POST'>\
                                    <input type='checkbox' name='valor' id='id_l6' onclick=\"processo(";
                                        pagina += "\'form_6\',";
                                        pagina += "\'id_l6\')";
                                        pagina += "\">";
                                        pagina += "\
                                    <input type='hidden' name='lampada' value='6'>\
                                </form>\
                            </div>\
                        </div>\
                    </div>\
                    <div class='linha'>\
                        <div class='linha_div' id='fundo7'>\
                            <div class='linha_div1'>\
                                <h2>";
                                    pagina += ler_nome(414);
                                    pagina += "\
                                </h2>\
                            </div>\
                            <div class='linha_div2'>\
                                <h3>";
                                    pagina += ler_horario(280);
                                    pagina += "-";
                                    pagina += ler_horario(282);
                                    pagina += "\
                                </h3>\
                            </div>\
                            <div class='linha_div3'>\
                                <form id='form_7' method='POST'>\
                                    <input type='checkbox' name='valor' id='id_l7' onclick=\"processo(";
                                        pagina += "\'form_7\',";
                                        pagina += "\'id_l7\')";
                                        pagina += "\">";
                                        pagina += "\
                                    <input type='hidden' name='lampada' value='7'>\
                                </form>\
                            </div>\
                        </div>\
                        <div class='linha_div' id='fundo8'>\
                            <div class='linha_div1'>\
                                <h2>";
                                    pagina += ler_nome(435);
                                    pagina += "\
                                </h2>\
                            </div>\
                            <div class='linha_div2'>\
                                <h3>";
                                    pagina += ler_horario(284);
                                    pagina += "-";
                                    pagina += ler_horario(286);
                                    pagina += "\
                                </h3>\
                            </div>\
                            <div class='linha_div3'>\
                                <form id='form_8' method='POST'>\
                                    <input type='checkbox' name='valor' id='id_l8' onclick=\"processo(";
                                        pagina += "\'form_8\',";
                                        pagina += "\'id_l8\')";
                                        pagina += "\">";
                                        pagina += "\
                                    <input type='hidden' name='lampada' value='8'>\
                                </form>\
                            </div>\
                        </div>\
                    </div>\
                    <div class='hora_config'>\
                        <h4>Hora Atual: ";
                            if (now.hour() < 10)
                            {
                                pagina += "0";
                                pagina += now.hour();
                            }
                            else
                            {
                                pagina += now.hour();
                            }
                            pagina += ":";
                            if (now.minute() < 10)
                            {
                                pagina += "0";
                                pagina += now.minute();
                            }
                            else
                            {
                                pagina += now.minute();
                            }
                            pagina += "\
                        </h4>\
                        <a href='/config'><i>Configurações</i></a>\
                    </div>\
                </section>\
                <p>\
                    <b>Info:</b> Durante o horário exato de ativação/desativação de uma lâmpada, a configuração manual da mesma não funcionará até que o relógio mude o horário (se passe pelo menos +1 minuto).\
                </p>\
            </main>\
            <script>";
                for (byte i = 1; i <= 8; i++)
                {
                    pagina += "var id_input = 'id_l";
                    pagina += i;
                    pagina += "';";
                    pagina += "var id_fundo = 'fundo";
                    pagina += i;
                    pagina += "';";
                    pagina += "\
                                            var input_r = document.getElementById(id_input);\
                                            var fundo_r = document.getElementById(id_fundo);";
                    if (rele[i - 1] == 1)
                    {
                        pagina += "input_r.checked = 1;";
                        pagina += "fundo_r.style.backgroundColor = '#09BA99';";
                    }
                    if (rele[i - 1] == 0)
                    {
                        pagina += "input_r.checked = 0;";
                        pagina += "fundo_r.style.backgroundColor = '#DF5352';";
                    }
                }
                if (p == 1)
                    pagina += "window.location.href = \"http://222.111.2.1\"";
                pagina += "\
            </script>\
        </body>\
     </html>";
    server.send(200, "text/html", pagina);
}

void html_config()
{
    DateTime now = rtc.now();
    boolean p = 0;
    if(server.hasArg("att")) att = 1;
    if (server.hasArg("lampada"))
    {
        p = 1;
        int inicial = 4 * (server.arg("lampada").toInt() - 1) + 256;
        if (server.arg("ligar") != "")
        {
            escrever_horario(server.arg("ligar"), inicial);
        }
        if (server.arg("desligar") != "")
        {
            escrever_horario(server.arg("desligar"), inicial + 2);
        }
        if (server.arg("nome") != "" && server.arg("nome").length() < 21)
        {
            escrever_nome(server.arg("nome"), pi_palavras[server.arg("lampada").toInt() - 1]);
        }
    }
    if (server.hasArg("hora"))
    {
        p = 1;
        escrever_hora(server.arg("hora"));
    }
    String pagConfig = "\
    <!DOCTYPE html>\
        <html lang='pt-br'>\
        <head>\
            <title>Configurações</title>\
            <meta charset='utf-8'>\
            <meta name='viewport' content='width=device-width, initial-scale=1'>\
            <script>\
                var matriz  =   [];\
                matriz[0]   =   [";
                pagConfig   +=  EEPROM.read(256);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(257);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(258);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(259);
                pagConfig   +=  "];";
                pagConfig   +=  "\
                matriz[1]   =   [";
                pagConfig   +=  EEPROM.read(260);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(261);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(262);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(263);
                pagConfig   +=  "];";
                pagConfig   +=  "\
                matriz[2]   =   [";
                pagConfig   +=  EEPROM.read(264);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(265);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(266);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(267);
                pagConfig   +=  "];";
                pagConfig   +=  "\
                matriz[3]   =   [";
                pagConfig   +=  EEPROM.read(268);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(269);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(270);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(271);
                pagConfig   +=  "];";
                pagConfig   +=  "\
                matriz[4]   =   [";
                pagConfig   +=  EEPROM.read(272);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(273);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(274);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(275);
                pagConfig   +=  "];";
                pagConfig   +=  "\
                matriz[5]   =   [";
                pagConfig   +=  EEPROM.read(276);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(277);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(278);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(279);
                pagConfig   +=  "];";
                pagConfig   +=  "\
                matriz[6]   =   [";
                pagConfig   +=  EEPROM.read(280);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(281);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(282);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(283);
                pagConfig   +=  "];";
                pagConfig   +=  "\
                matriz[7]   =   [";
                pagConfig   +=  EEPROM.read(284);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(285);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(286);
                pagConfig   +=  ",";
                pagConfig   +=  EEPROM.read(287);
                pagConfig   +=  "];";
                pagConfig   +=  "console.log(matriz);";
                pagConfig += "\
                function processo(formulario_id, input_ligar_id, input_desligar_id, lampada_id){\
                    horario_ligar = document.getElementById(input_ligar_id).value;\
                    horario_desligar = document.getElementById(input_desligar_id).value;\
                    lampada = document.getElementById(lampada_id).value;\
                    formulario = document.getElementById(formulario_id);\
                    if(horario_ligar != '' & horario_desligar != ''){\
                        hora_ligar = horario_ligar[0] + horario_ligar[1];\
                        minuto_ligar = horario_ligar[3] + horario_ligar[4];\
                        hora_desligar = horario_desligar[0] + horario_desligar[1];\
                        minuto_desligar = horario_desligar[3] + horario_desligar[4];\
                        if(horario_ligar == horario_desligar){\
                            alert('Horários iguais não são permitidos!');\
                            return false;\
                        }\
                        if(hora_ligar == matriz[lampada-1][2] & minuto_ligar == matriz[lampada-1][3] & hora_desligar == matriz[lampada-1][0] & minuto_desligar == matriz[lampada-1][1]){\
                            alert('Horários de acionamento e desacionamento ambos iguais aos já cadastrados!');\
                            return false;\
                        }\
                        if(hora_ligar == matriz[lampada-1][2] & minuto_ligar == matriz[lampada-1][3]){\
                            alert('Horário de acionamento igual ao de desacionamento já cadastrado!');\
                            return false;\
                        }\
                        if(hora_desligar == matriz[lampada-1][0] & minuto_desligar == matriz[lampada-1][1]){\
                            alert('Horário de desacionamento igual ao de acionamento já cadastrado!');\
                            return false;\
                        }\
                    }\
                    if(horario_ligar != '' & horario_desligar == ''){\
                        hora_ligar = horario_ligar[0] + horario_ligar[1];\
                        minuto_ligar = horario_ligar[3] + horario_ligar[4];\
                        if(hora_ligar == matriz[lampada-1][2] & minuto_ligar == matriz[lampada-1][3]){\
                            alert('Horário de acionamento igual ao de desacionamento já cadastrado!');\
                            return false;\
                        }\
                    }\
                    if(horario_ligar == '' & horario_desligar != ''){\
                        hora_desligar = horario_desligar[0] + horario_desligar[1];\
                        minuto_desligar = horario_desligar[3] + horario_desligar[4];\
                        if(hora_desligar == matriz[lampada-1][0] & minuto_desligar == matriz[lampada-1][1]){\
                            alert('Horário de desacionamento igual ao de acionamento já cadastrado!');\
                            return false;\
                        }\
                    }\
                    formulario.submit();\
                }\
            </script>\
            <style>\
                *{\
                    padding: 0;\
                    margin: 0;\
                    text-decoration: none;\
                    font-family: Trebuchet MS, sans-serif;\
                }\
                main{\
                    display: flex;\
                    flex-direction: column;\
                    align-items: center;\
                }\
                h1{\
                    text-decoration: underline;\
                    margin-top: 4%;\
                    margin-bottom: 2%;\
                    font-size: 28px;\
                    height: 5vh;\
                }\
                h3{\
                    margin-top: 0.5%;\
                    margin-bottom: 0.5%;\
                    text-decoration: underline;\
                }\
                h4{\
                    background-color: rgb(255, 203, 203);\
                    border-radius: 10px;\
                    padding: 5px;\
                    margin-bottom: 1%;\
                    font-weight: 1;\
                    font-weight: 400;\
                }\
                label{\
                    font-weight: 100;\
                }\
                .lampada, .hora {\
                    display: flex;\
                    flex-direction: column;\
                    align-items: center;\
                    border-radius: 30px;\
                    background-color: rgb(240, 240, 240);\
                    box-shadow: 0px 0px 7px black;\
                    margin-bottom: 5%;\
                    width: 94vw;\
                }\
                .lampada{\
                    height:50vh;\
                }\
                .hora{\
                    height:24vh;\
                }\
                form{\
                    display: flex;\
                    flex-direction: column;\
                    justify-content: space-evenly;\
                    align-items: center;\
                    width: 50%;\
                    height: 90%;\
                }\
                input, select{\
                    height: 30px;\
                }\
                input[type=time]{\
                    border-radius: 10px;\
                    border-style: solid;\
                    border-color: lightseagreen;\
                    width: 70px;\
                }\
                input[type=submit]{\
                    width: 80%;\
                    border-radius: 20px;\
                    border-style: none;\
                    background-color: rgb(158, 255, 113);\
                    cursor: pointer;\
                }\
                input[type=button]{\
                    width: 80%;\
                    border-radius: 20px;\
                    border-style: none;\
                    background-color: rgb(158, 255, 113);\
                    cursor: pointer;\
                }\
                .ancoras{\
                    display: flex;\
                    flex-direction: row;\
                    justify-content: space-between;\
                    margin-top: 2%;\
                    width: 100%;\
                }\
                a{\
                    background-color: rgb(255, 203, 203);\
                    border-radius: 10px;\
                    padding: 4px;\
                    font-size: 24px;\
                    color: black;\
                }\
                #a1{\
                    margin-left: 2%;\
                }\
                p{\
                    margin: 20px;\
                    flex-wrap: wrap;\
                }\
                @media screen and (min-width: 1000px){\
                    h3{\
                        font-size: 20px;\
                    }\
                    .lampada{\
                        width: 70vw;\
                        height: 65vh;\
                    }\
                    .hora{\
                        width: 70vw;\
                        height: 35vh;\
                    }\
                    input[type=submit]{\
                        width: 50%;\
                        cursor: pointer;\
                    }\
                    input[type=button]{\
                        width: 50%;\
                        cursor: pointer;\
                    }\
                    label{\
                        font-size: 19px;\
                    }\
                }\
            </style>\
        </head>\
        <body>\
            <main>\
                <div class='ancoras'>\
                    <div id='a1'><a href='../'><i>Voltar</i></a></div>\
                </div>\
                <h1>Configurações</h1>\
                <section class='lampada'>\
                    <h3>Configurar Lâmpada</h3>\
                    <form method='POST' id='id_form'>\
                        <label for='lamp'>Selecionar Lâmpada</label>\
                        <select id='lamp' name='lampada' required>";
                            for (byte i = 1; i <= 8; i++)
                            {
                                pagConfig += "<option value=\"";
                                pagConfig += i;
                                pagConfig += "\">";
                                pagConfig += ler_nome(pi_palavras[i - 1]);
                                pagConfig += "</option>";
                            }
                            pagConfig += "\
                        </select>\
                        <label for='name'>Novo nome</label>\
                        <input type='text' id='name' name='nome' placeholder='Opcional' maxlength='20'>\
                        <label for='lig'>Acionamento</label>\
                        <input type='time' id='lig' name='ligar'>\
                        <label for='des'>Desligamento</label>\
                        <input type='time' id='des' name='desligar'>\
                        <input type='button' value='Enviar' onclick=\"processo(";
                        pagConfig += "\'id_form\',";
                        pagConfig += "\'lig\',";
                        pagConfig += "\'des\',";
                        pagConfig += "\'lamp\')";
                        pagConfig += "\">";
                        pagConfig += "\
                    </form>\
                </section>\
                <section class='hora'>\
                    <h3>Configurar Hora</h3>\
                    <form method='POST'>\
                        <label for='hora'>Nova hora</label>\
                        <input type='time' id='hour' name='hora' required>\
                        <input type='hidden' name='att' value'1'>\
                        <input type='submit'>\
                    </form>\
                    <h4>Hora Atual: ";
                        if (now.hour() < 10)
                        {
                            pagConfig += "0";
                            pagConfig += now.hour();
                        }
                        else
                        {
                            pagConfig += now.hour();
                        }
                        pagConfig += ":";
                        if (now.minute() < 10)
                        {
                            pagConfig += "0";
                            pagConfig += now.minute();
                        }
                        else
                        {
                            pagConfig += now.minute();
                        }
                        pagConfig += "\
                    </h4>\
                </section>\
                <p>\
                    <b>Info: </b>Só serão salvos nomes com até 20 caracteres. Nomes com mais do que isso, serão considerados apenas os primeiros 20 caracteres.\
                </p>\
            </main>\
        </body>";
    if (p == 1 || att != 0)
    {
        if(att != 0 && att < 3) ++att;
        else att = 0;
        pagConfig += "<script>";
        pagConfig += "window.location.href = \"http://222.111.2.1/config\"";
        pagConfig += "</script>";
    }
    pagConfig += "</html>";
    server.send(200, "text/html", pagConfig);
}
