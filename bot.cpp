#include <iostream>
#include <curl/curl.h>
#include <string>
#include <thread>
#include <chrono>
#include "json.hpp" // JSON library
#include <curl/easy.h> // Header untuk URL Encoding

using namespace std;
using json = nlohmann::json;

const string BOT_TOKEN = "7663389256:AAGvz2CrFrdsUD1AUuOdc2dSodxG87_fFIk"; // Ganti dengan token bot Anda
const string API_URL = "https://api.telegram.org/bot" + BOT_TOKEN;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string urlEncode(const string &value) {
    CURL *curl = curl_easy_init();
    char *encoded = curl_easy_escape(curl, value.c_str(), value.length());
    string encoded_str(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    return encoded_str;
}

void sendMessage(const string& chat_id, const string& text) {
    CURL* curl = curl_easy_init();
    if (curl) {
        string encoded_text = urlEncode(text);
        string url = API_URL + "/sendMessage?chat_id=" + chat_id + "&text=" + encoded_text;

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

void botLoop() {
    string last_update_id = "0"; // Offset untuk memastikan hanya update terbaru yang diproses

    while (true) {
        CURL* curl = curl_easy_init();
        if (curl) {
            string response;
            string url = API_URL + "/getUpdates?offset=" + last_update_id; // Menambahkan offset untuk update baru
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            cout << "Response from getUpdates: " << response << endl; // Debugging

            if (!response.empty()) {
                try {
                    auto data = json::parse(response);
                    for (auto& update : data["result"]) {
                        // Update terakhir
                        last_update_id = to_string(update["update_id"].get<int>() + 1);

                        if (update.contains("message") && update["message"].contains("chat")) {
                            string chat_id = to_string(update["message"]["chat"]["id"].get<int>());
                            string text = update["message"]["text"].get<string>();

                            cout << "Received message: " << text << " from chat_id: " << chat_id << endl;

                            // Cek dan kirim pesan sesuai dengan perintah
                            if (text == "/on") {
                                sendMessage(chat_id, "Bot is now ON!");
                            } else if (text == "/off") {
                                sendMessage(chat_id, "Bot is now OFF!");
                            } else {
                                sendMessage(chat_id, "Unknown command: " + text);
                            }
                        }
                    }
                } catch (const json::exception& e) {
                    cerr << "JSON parsing error: " << e.what() << endl;
                }
            }
        }
        this_thread::sleep_for(chrono::seconds(1)); // Cek pembaruan setiap 1 detik
    }
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    cout << "Bot is starting..." << endl;
    botLoop();
    curl_global_cleanup();
    return 0;
}
