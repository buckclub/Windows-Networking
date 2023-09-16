#include "Networking.h"

int main() {

	nlohmann::json webhookData;
	nlohmann::json embed;
	nlohmann::json author;
	author["name"] = "Mr. Author";
	embed["title"] = "Title test";
	embed["description"] = "This is a long description for example purposes.";
	embed["color"] = 16711680;
	embed["author"] = author;
	embed["image"]["url"] = "https://i.imgur.com/APkqZLq.jpeg";
	webhookData["embeds"] = nlohmann::json::array({ embed });

	const char* webhookUrl = ""; // Format: https:// discord.com/api/webhooks/(webhook id)/(webhook token)
	Networking::GetInstance().SendDiscordWebhookMessage(webhookUrl, webhookData);

	return 0;

}