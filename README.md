# Documentação do Projeto Monitoramento de Planta (Green House Monitoring)

## Objetivo

O objetivo deste projeto é desenvolver um sistema IoT para medições precisas de uma planta e exibição dessas informações em um painel web. O sistema permitirá monitorar a umidade do solo, a temperatura e umidade do ambiente, bem como a intensidade da luz, auxiliando no cuidado e manutenção das plantas.

## Componentes Utilizados

1. **Microcontrolador ESP32**: Coleta dados dos sensores e envia para o painel web.
2. **Sensor de Umidade e Temperatura DHT11**: Mede a umidade e temperatura do ambiente.
3. **Sensor de Umidade do Solo**: Mede a umidade do solo onde a planta está plantada.
4. **LDR (Light Dependent Resistor)**: Mede a intensidade da luz no ambiente.

## Arquitetura do Sistema

1. **Sensores**: Capturam dados de temperatura, umidade do ar, umidade do solo e intensidade da luz.
2. **Microcontrolador ESP32**: Processa os dados dos sensores e os envia para o servidor.
3. **Servidor Web**: Recebe os dados do ESP32 e exibe em um painel web acessível via navegador.

## Funcionalidades

- **Medição da Temperatura e Umidade do Ar**: O sensor DHT11 captura a temperatura e a umidade do ambiente.
- **Medição da Umidade do Solo**: O sensor de umidade do solo fornece informações sobre o nível de umidade da terra.
- **Medição da Intensidade da Luz**: O LDR captura a intensidade da luz no ambiente onde a planta está localizada.
- **Exibição dos Dados**: Um painel web apresenta todas as medições de forma clara e acessível.

## Descrição

Este projeto oferece uma base sólida para o monitoramento de plantas utilizando IoT. Pode ser expandido com mais sensores ou funcionalidades conforme necessário. A simplicidade do sistema facilita o entendimento e a implementação, sendo ideal para iniciantes em IoT e automação de jardinagem.

## Equipe

- Beni
- Cauan
- Luiz

## Recursos Adicionais

- [Site](https://greenhouse-monitoring.netlify.app)
- [Diagrama do Sistema](https://greenhouse-monitoring.netlify.app/diagrama)