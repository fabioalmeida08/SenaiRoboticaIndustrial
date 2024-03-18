import requests
import boto3
import json


class Telegram_bot:
    def __init__(self, event):
        self.bot_token_parameter = "/BotToken/Senai"
        self.chat_id_parameter = "ChatID"
        self.uuid = (
            event.get("Records", [{}])[0]
            .get("dynamodb", {})
            .get("Keys", {})
            .get("id", {})
            .get("S")
        )
        self.date = self.date = (
            event.get("Records", [{}])[0]
            .get("dynamodb", {})
            .get("NewImage", {})
            .get("date", {})
            .get("S")
        )
        self.success = {
            "statusCode": 201,
            "body": json.dumps({"status": "mensagem enviada com successo"}),
        }
        self.error = {"statusCode": 400, "body": json.dumps("erro ao enviar mensagem")}

    def send_telegram_message(self, message):
        ssm = boto3.client("ssm")

        bot_token_parameter = ssm.get_parameter(
            Name=self.bot_token_parameter, WithDecryption=True
        )
        bot_token = bot_token_parameter["Parameter"]["Value"]

        chat_id_parameter = ssm.get_parameter(
            Name=self.chat_id_parameter, WithDecryption=True
        )
        chat_id = chat_id_parameter["Parameter"]["Value"]

        url = f"https://api.telegram.org/bot{bot_token}/sendMessage"

        data = {
            "chat_id": chat_id,
            "text": message,
            # "reply_markup": reply_markup
        }

        response = requests.post(url, json=data)
        print(response.text)
        if response.ok:
            return self.success
        else:
            return self.error

    def process_event(self):
        message = f"peça registrada : {self.uuid}, em {self.date}"
        return self.send_telegram_message(message=message)
