#include <iostream>
#include <curl/curl.h>
#include <string>
#include <thread>
#include <chrono>
#include "json.hpp" // JSON library

using namespace std;
using json = nlohmann::json;

// Bot Token dari BotFather
const string BOT_TOKEN = "7663389256:AAGvz2CrFrdsUD1AUuOdc2dSodxG87_fFIk"; // Ganti dengan token bot Anda
const string API_URL = "https://api.telegram.org/bot" + BOT_TOKEN;

// Status bot
bool bot_status = false;

// Fungsi untuk meng-handle response dari libcurl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Fungsi untuk mengirim pesan ke chat
void sendMessage(const string& chat_id, const string& text) {
    CURL* curl = curl_easy_init();
    if (curl) {
        string url = API_URL + "/sendMessage?chat_id=" + chat_id + "&text=" + curl_easy_escape(curl, text.c_str(), text.length());

        string response;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "Curl error: " << curl_easy_strerror(res) << endl;
        } else {
            cout << "Response: " << response << endl;
        }

        curl_easy_cleanup(curl);
    }
}

// Fungsi utama untuk loop bot
void botLoop() {
    string last_update_id = "0";

    while (true) {
        CURL* curl = curl_easy_init();
        if (curl) {
            string response;
            string url = API_URL + "/getUpdates?offset=" + last_update_id;

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK && !response.empty()) {
                try {
                    auto data = json::parse(response);
                    for (auto& update : data["result"]) {
                        last_update_id = to_string(update["update_id"].get<int>() + 1);
                        string chat_id = to_string(update["message"]["chat"]["id"].get<int>());
                        string text = update["message"]["text"].get<string>();

                        cout << "Received message: " << text << " from chat_id: " << chat_id << endl;

                        // Handle perintah
                        if (text == "/start") {
                            sendMessage(chat_id, "Bot is running! Use /on or /off to control the bot.");
                        } else if (text == "/on") {
                            if (bot_status) {
                                sendMessage(chat_id, "Bot is already ON!");
                            } else {
                                bot_status = true;
                                sendMessage(chat_id, "Bot is now ON!");
                            }
                        } else if (text == "/off") {
                            if (!bot_status) {
                                sendMessage(chat_id, "Bot is already OFF!");
                            } else {
                                bot_status = false;
                                sendMessage(chat_id, "Bot is now OFF!");
                            }
                        } else {
                            sendMessage(chat_id, "Unknown command: " + text);
                        }
                    }
                } catch (const json::exception& e) {
                    cerr << "JSON parsing error: " << e.what() << endl;
                }
            } else {
                cerr << "Curl error: " << curl_easy_strerror(res) << endl;
            }

            curl_easy_cleanup(curl);
        }

        // Delay 1 detik sebelum polling ulang
        this_thread::sleep_for(chrono::seconds(1));
    }
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    cout << "Bot is starting..." << endl;
    botLoop();
    curl_global_cleanup();
    return 0;
}
