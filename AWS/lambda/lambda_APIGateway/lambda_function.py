import json
import boto3


def lambda_handler(event, context):
    if "body" in event:
        # acessar o corpo JSON da solicitação
        json_body = json.loads(event["body"])

        # pegar o campo uuid gerado pela requisição do dispositivo NodeMCU
        uuid = json_body.get("uuid")

        # criar um client ssm para poder acessar as variáveis cryptografadas pelo parameter store
        ssm = boto3.client("ssm")
        sqs_url_parameter = ssm.get_parameter(Name="/Senai/SQS", WithDecryption=True)
        sqs_url = sqs_url_parameter["Parameter"]["Value"]

        # conectar-se ao cliente SQS
        sqs_client = boto3.client("sqs")

        # definir o corpo da mensagem para enviar para a fila SQS
        message_body = json.dumps(json_body)

        # enviar a mensagem para a fila SQS
        response = sqs_client.send_message(QueueUrl=sqs_url, MessageBody=message_body)

        # retornar uma resposta indicando que a mensagem foi enviada com sucesso para a fila SQS
        return {
            "statusCode": 200,
            "body": json.dumps("Mensagem enviada com sucesso para a fila SQS"),
        }
    else:
        # se não houver corpo na solicitação, retornar uma resposta de erro
        return {
            "statusCode": 400,
            "body": json.dumps("Corpo da solicitação não encontrado"),
        }
