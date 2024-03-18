import requests
import boto3
import json


class Telegram_bot:
    def __init__(self, command):
        self.command = command
        self.bot_token_parameter = "/BotToken/Senai"
        self.chat_id_parameter = "ChatID"
        self.success = {
            "statusCode": 200,
            "body": json.dumps({"status": "mensagem recebida"}),
        }
        self.error = {
            "statusCode": 500,
            "body": json.dumps("erro ao processar mensagem"),
        }

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
        if self.command == "/listar10":

            dynamodb = boto3.client("dynamodb")

            # pegar o nome da tabela do ssm
            ssm = boto3.client("ssm")
            senai_table_name_parameter = ssm.get_parameter(Name="/Senai/DynamoDB")
            senai_table = senai_table_name_parameter["Parameter"]["Value"]

            # estou usando o scan ao em vez
            # de query, mesmo sabendo que não é o método mais recomendado,
            # pois quero manter o projeto dentro do free tier, e criar um GSI ou LSI
            # incorreria em custos adicionais.
            response = dynamodb.scan(
                TableName=senai_table,
            )

            items = response["Items"]

            formatted_items = [
                {"id": item["id"]["S"], "date": item["date"]["S"]} for item in items
            ]

            # ordene os itens pela data
            sorted_items = sorted(
                formatted_items, key=lambda x: x["date"], reverse=True
            )

            return self.send_telegram_message(message=sorted_items)
