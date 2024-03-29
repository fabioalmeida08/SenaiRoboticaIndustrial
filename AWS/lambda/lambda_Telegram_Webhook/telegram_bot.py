import boto3
import requests
import json
from boto3.dynamodb.conditions import Key


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
        if response.ok:
            return self.success
        else:
            return self.error

    def initialize_dynamodb(self):
        dynamodb = boto3.resource("dynamodb")

        # pegar o nome da tabela do SSM
        ssm = boto3.client("ssm")
        senai_table_name_parameter = ssm.get_parameter(Name="/Senai/DynamoDB")
        senai_table = senai_table_name_parameter["Parameter"]["Value"]

        return dynamodb.Table(senai_table)

    def process_event(self):
        try:
            # dividir o comando para pegar os paremetros passados pelo user
            par = self.command.split()

            # tornar a primeira letra de cada parametro maiúscula
            for i in range(len(par)):
                par[i] = par[i].capitalize()

            # criaçao dos arrays com os valores válidos para validação
            materials = ["Metal", "Polimero"]
            sizes = ["Pequeno", "Medio", "Grande"]

            # validação do comando
            if len(par) < 2 or (
                par[0] == "/s" and len(par) == 2 and par[1] not in materials
            ):
                message = "Comando inválido:\nuse /s acompanhado do material ou material e tamanho\nEx: /s metal ou /s polimero pequeno"
                return self.send_telegram_message(message)
            elif par[0] == "/s":
                if len(par) == 3 and (par[1] not in materials or par[2] not in sizes):
                    message = "Comando inválido:\nMaterial ou tamanho inválido\nMateriais válidos: metal, polimero\nTamanhos válidos: pequeno, medio, grande"
                    return self.send_telegram_message(message)
            elif len(par) > 3 or len(par) == 1 and par[0] == "/s":
                message = "Comando inválido:\nuse /s acompanhado do material ou material e tamanho\nEx: /s metal ou /s polimero pequeno"
                return self.send_telegram_message(message)

            table = self.initialize_dynamodb()
            all_items = []

            # token de paginação inicial
            pagination_token = None

            # comparaçao dos valores dos parametros
            if (
                len(par) == 3
                and par[0] == "/s"
                and par[1] in materials
                and par[2] in sizes
            ):
                try:
                    # paginação
                    while True:
                        if pagination_token is None:
                            response = table.query(
                                IndexName="material-size-index",
                                KeyConditionExpression=Key("material").eq(par[1])
                                & Key("size").eq(par[2]),
                                ScanIndexForward=False,
                            )
                        else:
                            response = table.query(
                                IndexName="material-size-index",
                                KeyConditionExpression=Key("material").eq(par[1])
                                & Key("size").eq(par[2]),
                                ScanIndexForward=False,
                                ExclusiveStartKey=pagination_token,
                            )

                        # adicionando os itens recuperados na lista
                        all_items.extend(response["Items"])

                        # verificaçao para paginação , caso exista mais items
                        if "LastEvaluatedKey" in response:
                            pagination_token = response["LastEvaluatedKey"]
                        else:
                            break
                except Exception as e:
                    print("erro --->", e)
                    # retornando sucesso mesmo no caso de erro
                    # para debugar pois o bot do telegram ficará tentando enviar a mensagem até receber um status 200
                    return self.success
            elif len(par) == 2 and par[0] == "/s" and par[1] in materials:
                try:
                    while True:
                        if pagination_token is None:
                            response = table.query(
                                IndexName="material-size-index",
                                KeyConditionExpression=Key("material").eq(par[1]),
                                ScanIndexForward=False,
                            )
                        else:
                            response = table.query(
                                IndexName="material-size-index",
                                KeyConditionExpression=Key("material").eq(par[1]),
                                ScanIndexForward=False,
                                ExclusiveStartKey=pagination_token,
                            )

                        all_items.extend(response["Items"])

                        if "LastEvaluatedKey" in response:
                            pagination_token = response["LastEvaluatedKey"]
                        else:
                            break
                except Exception as e:
                    print("erro --->", e)
                    # retornando sucesso mesmo no caso de erro
                    # para debugar pois o bot do telegram ficará tentando enviar a mensagem até receber um status 200
                    return self.success

            # organizar o retorno dos ultimos 5 items e formatar para a mensagem do bot
            sorted_by_date = sorted(all_items, key=lambda x: x["date"], reverse=True)
            last_five = sorted_by_date[:5]
            formated_res = []
            for item in last_five:
                piece = f"ID: {item['id']} \nMaterial: {item['material']} \nTamanho: {item['size']}\nLote: {item['lote']} \nData: {item['date']}\n"
                formated_res.append(piece)
            message = "\n".join(formated_res)
            return self.send_telegram_message(message=message)
        except Exception as e:
            print("erro --->", e)
            # retornando sucesso mesmo no caso de erro
            # para debugar pois o bot do telegram ficará tentando enviar a mensagem até receber um status 200
            return self.success
