#include <iostream>
#include <curl/curl.h>
#include <string>
#include <thread>
#include <chrono>
#include "json.hpp" // JSON library

using namespace std;
using json = nlohmann::json;

const string BOT_TOKEN = "123456789:ABCdefGhijKLMnopQrstUVwxYZ"; // Ganti dengan token bot Anda
const string API_URL = "https://api.telegram.org/bot" + BOT_TOKEN;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void sendMessage(const string& chat_id, const string& text) {
    CURL* curl = curl_easy_init();
    if (curl) {
        string url = API_URL + "/sendMessage?chat_id=" + chat_id + "&text=" + text;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
}

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
            curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            auto data = json::parse(response);
            for (auto& update : data["result"]) {
                last_update_id = to_string(update["update_id"].get<int>() + 1);
                string chat_id = update["message"]["chat"]["id"].get<string>();
                string text = update["message"]["text"].get<string>();

                cout << "Received message: " << text << endl;
                if (text == "/on") {
                    sendMessage(chat_id, "Bot is now ON!");
                } else if (text == "/off") {
                    sendMessage(chat_id, "Bot is now OFF!");
                } else {
                    sendMessage(chat_id, "Unknown command: " + text);
                }
            }
        }
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
