from telegram_bot import Telegram_bot


def lambda_handler(event, context):
    bot = Telegram_bot(event)
    bot.process_event()
