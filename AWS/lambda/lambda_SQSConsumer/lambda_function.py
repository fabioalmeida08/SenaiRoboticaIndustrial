import json
import boto3
import datetime
import pytz


def lambda_handler(event, context):
    # formatar a data
    fuso_horario_brasil = pytz.timezone("America/Sao_Paulo")
    data_hora_brasil = datetime.datetime.now(fuso_horario_brasil)
    formato = "%d-%m-%Y %H:%M:%S"
    data_hora_formatada = data_hora_brasil.strftime(formato)

    # extrair o uuid da fila sqs
    uuid = json.loads(event["Records"][0].get("body")).get("uuid")

    # inicialização do cliente DynamoDB
    dynamodb = boto3.resource("dynamodb")

    # pegar o nome da tabela do ssm
    ssm = boto3.client("ssm")
    senai_table_name_parameter = ssm.get_parameter(Name="/Senai/DynamoDB")
    senai_table = senai_table_name_parameter["Parameter"]["Value"]

    # pegar referencia para a tabela ddb
    table = dynamodb.Table(senai_table)

    # dados a serem registrados no dynamodb
    item_data = {"id": uuid, "date": data_hora_formatada}

    try:
        # registrar o item na tabela
        response = table.put_item(Item=item_data)
        print("Item registrado com sucesso:", response)
        return {
            "statusCode": 200,
            "body": json.dumps("Item registrado com sucesso no DynamoDB"),
        }
    except Exception as e:
        print("Erro ao registrar item:", e)
        return {
            "statusCode": 500,
            "body": json.dumps("Erro ao registrar item no DynamoDB"),
        }
