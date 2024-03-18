import json
import boto3
from telegram_bot import Telegram_bot


def lambda_handler(event, context):
    body = json.loads(event["body"])

    command = body["message"]["text"]

    bot = Telegram_bot(command)
    bot.process_event()

    return {"statusCode": 200, "body": json.dumps("Mensagem recebida com sucesso")}
