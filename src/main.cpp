// Libraries used in this project
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <UniversalTelegramBot.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

// Define wifi credentials, Bot token and a local host name
#include <credentials.h>
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASS;
const char *host = MDNS_HOST;
const char *bot_token = BOT_TOKEN;

// Variable for storing the states
int q1 = 0;

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// Define objects
WebServer server(80);
WiFiClientSecure client;
UniversalTelegramBot bot(bot_token, client);

// Store the webpage html and css code
String style;
String loginIndex;
String serverIndex;

// Callback function called when a new message arrived
void handleNewMessages(int numNewMessages) {
    Serial.print("\n\n New Telegram message received: ");

    // Handle any number of messages in the queue
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = bot.messages[i].chat_id;
        String text = bot.messages[i].text;
        String from_name = bot.messages[i].from_name;

        if (from_name == "") {
            from_name = "Guest";
        }

        Serial.println(text);
        text.toLowerCase();

        // Inline buttons with callbacks when pressed will raise a callback_query message
        if (bot.messages[i].type == "callback_query") {
            if (text == "q1_1") {
                q1 = 1;
            }
            if (text == "q1_2") {
                q1 = 2;
            }
            if (text == "q1_3") {
                q1 = 3;
            }
            if (text == "q1_4") {
                q1 = 4;
            }
            if (text == "q1_5") {
                q1 = 5;
            }
            // Handle commands
        } else {
            if (strstr("/start help hello", text.c_str())) {
                String welcome =
                    "Salut.\n"
                    "Bine ai venit la cabinetul meu virtual de consiliere.\n\n"
                    "Numele meu este Anda Faur si te invit sa alegi una din urmatoarele optiuni.\n\n"
                    "/pda_screening  : Completeaza chestionarul PDA. \n"
                    "/rezultate      : Genereaza rezultatele dupa completare.";
                bot.sendMessage(chat_id, welcome, "");
            }

            if (text == "/pda_screening") {
                bot.sendChatAction(chat_id, "typing");
                delay(500);

                String welcome =
                    "Mai jos vă prezentăm o listă de cuvinte care descriu emoţiile pe care oamenii le au în diverse situaţii. \n"
                    "Citiţi cu atenţie fiecare cuvânt, apoi marcaţi varianta care corespunde cel mai bine întrebării:";

                bot.sendMessage(chat_id, welcome, "");

                bot.sendChatAction(chat_id, "typing");
                delay(500);

                welcome = "CUM V-AŢI SIMŢIT ÎN ULTIMELE DOUĂ SĂPTĂMÂNI?";
                bot.sendMessage(chat_id, welcome, "");
                bot.sendChatAction(chat_id, "typing");
                delay(500);

                welcome = "Obosit";

                String keyboardJson =
                    "["
                    "[{ \"text\" : \"Deloc\", \"callback_data\" : \"q1_1\" }],"
                    "[{ \"text\" : \"Foarte putin\", \"callback_data\" : \"q1_2\" }],"
                    "[{ \"text\" : \"Mediu\", \"callback_data\" : \"q1_3\" }],"
                    "[{ \"text\" : \"Mult\", \"callback_data\" : \"q1_4\" }],"
                    "[{ \"text\" : \"Foarte mult\", \"callback_data\" : \"q1_5\" }]"
                    "]";

                //String keyboardJson = "[[{ \"text\" : \"Go to Google\", \"url\" : \"https://www.google.com\" }],[{ \"text\" : \"Send\", \"callback_data\" : \"This was sent by inline\" }]]";
                bot.sendMessageWithInlineKeyboard(chat_id, welcome, "", keyboardJson);
                Serial.println("inline message sent");
            }

            if (text == "/rezultate") {
                bot.sendChatAction(chat_id, "typing");
                delay(100);
                String welcome = "Scorul obtinut de dumneavoastra este: ";
                welcome += String(q1);
                bot.sendMessage(chat_id, welcome, "");
            }

            if (strstr("site cv", text.c_str())) {
                String keyboardJson =
                    "[[{ \"text\" : \"Go to a website\", \"url\" : \"https://timandi.xyz\" }],"
                    "[{ \"text\" : \"Tell him I said Hi\", \"callback_data\" : \"Si el :))\" }]]";
                bot.sendMessageWithInlineKeyboard(chat_id, "Choose from one of the following options", "", keyboardJson);
            }

            if (text == "/joke") {
                bot.sendChatAction(chat_id, "typing");
                delay(1000);
                bot.sendMessage(chat_id, ":|", "");
            }
        }
    }
}
/**====================================
 *    WIFI & OTA setup function
 *===================================**/
void setupWifi() {
    // CSS style
    style =
        "<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
        "input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"
        "#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
        "#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
        "form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
        ".btn{background:#3498db;color:#fff;cursor:pointer}</style>";

    // Login page
    loginIndex =
        "<form name=loginForm>"
        "<h1>PsihoBot Login</h1>"
        "<input name=userid placeholder='User ID'> "
        "<input name=pwd placeholder=Password type=Password> "
        "<input type=submit onclick=check(this.form) class=btn value=Login></form>"
        "<script>"
        "function check(form) {"
        "if(form.userid.value=='admin' && form.pwd.value=='admin')"
        "{window.open('/serverIndex')}"
        "else"
        "{alert('Error Password or Username')}"
        "}"
        "</script>" +
        style;

    // Update page
    serverIndex =
        "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
        "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
        "<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
        "<label id='file-input' for='file'>   Choose file...</label>"
        "<input type='submit' class=btn value='Update'>"
        "<br><br>"
        "<div id='prg'></div>"
        "<br><div id='prgbar'><div id='bar'></div></div><br></form>"
        "<script>"
        "function sub(obj){"
        "var fileName = obj.value.split('\\\\');"
        "document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
        "};"
        "$('form').submit(function(e){"
        "e.preventDefault();"
        "var form = $('#upload_form')[0];"
        "var data = new FormData(form);"
        "$.ajax({"
        "url: '/update',"
        "type: 'POST',"
        "data: data,"
        "contentType: false,"
        "processData:false,"
        "xhr: function() {"
        "var xhr = new window.XMLHttpRequest();"
        "xhr.upload.addEventListener('progress', function(evt) {"
        "if (evt.lengthComputable) {"
        "var per = evt.loaded / evt.total;"
        "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
        "$('#bar').css('width',Math.round(per*100) + '%');"
        "}"
        "}, false);"
        "return xhr;"
        "},"
        "success:function(d, s) {"
        "console.log('success!') "
        "},"
        "error: function (a, b, c) {"
        "}"
        "});"
        "});"
        "</script>" +
        style;

    // Additional SSL certificate
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

    // Connect to WiFi network
    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Use MDSN for a friendly hostname - http://psihobot.local
    if (!MDNS.begin(host)) {
        Serial.println("Error setting up MDNS responder!");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("You can access the dashboard at http://psihobot.local");

    // Returns login page
    server.on("/", HTTP_GET, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", loginIndex);
    });

    // Returns server page
    server.on("/serverIndex", HTTP_GET, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", serverIndex);
    });

    // Handle upload of new firmware
    server.on(
        "/update", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart(); }, []() {
            HTTPUpload& upload = server.upload();
            if (upload.status == UPLOAD_FILE_START) 
            {
                Serial.printf("Update: %s\n", upload.filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) 
                { 
                    Update.printError(Serial);
                }
            } 
            else if (upload.status == UPLOAD_FILE_WRITE) 
            {
                // Flashing firmware to ESP
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) 
                {
                    Update.printError(Serial);
                }
            } 
            else if (upload.status == UPLOAD_FILE_END) 
            {
                if (Update.end(true)) 
                { 
                    Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                } 
                else 
                {
                    Update.printError(Serial);
                }
            } });
    server.begin();
    Serial.println("All good ;)");
}

/**====================================
 *    SETUP function
 *===================================**/
void setup(void) {
    Serial.begin(115200);

    setupWifi();
}

/**====================================
 *    LOOP function
 *===================================**/
void loop(void) {
    server.handleClient();
    delay(1);

    if (millis() > lastTimeBotRan + botRequestDelay) {
        // Check for new messages
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

        while (numNewMessages) {
            Serial.println("got response");
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        lastTimeBotRan = millis();
    }
}
