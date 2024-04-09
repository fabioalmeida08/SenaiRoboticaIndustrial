
# Trabalho do Curso de Robótica Industrial Senai

![imagem_arquitetura](./arquitetura_serverless_aws.png "arquitetura serveless")

## integração com a Esteira e o CB3
https://github.com/fabioalmeida08/SenaiRoboticaIndustrial/assets/91635002/e2af3b2e-c791-4f21-a8f7-2d7d5c425506

Toda vez que uma peça passa pelos leitores da esteira, esse sinal é enviado à placa que monta a estrutura a ser enviada para a arquitetura AWS.
A placa também esta integrada com a plataform da Blynk que mostra informação do total de peças que passaram pelo sensor junto com o tamanho.

OBS: é possível ver a notificação do telegram em 40s do video.
### Integração com API do Telegram
#### Recebimento das mensagems
https://github.com/fabioalmeida08/SenaiRoboticaIndustrial/assets/91635002/c1eece0f-74c3-4a23-9588-12d1d96e31a0

https://github.com/fabioalmeida08/SenaiRoboticaIndustrial/assets/91635002/a7ba8ece-9185-4cb9-a5ad-eacc215e6937
#### Comandos do bot
https://github.com/fabioalmeida08/SenaiRoboticaIndustrial/assets/91635002/c56ebadb-460d-4619-922b-0712a75c0714



### lcd acoplado a placa mostrando informações da peça
https://github.com/fabioalmeida08/SenaiRoboticaIndustrial/assets/91635002/5108b730-72b5-47b9-831c-705459159f65


## Todo-list

- [x] substituir o botão pela leitura do sensor óptico da esteira
- [x] refatorar o código com o design de sensores da esteira
- [x] acionar a o robô remotamente via esp8266
