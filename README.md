# Smart-Dispenser
Smart Dispenser IoT 🩺💊

Um dispensador inteligente de medicamentos com ESP32, LCD I2C, servo motor, RTC DS3231, sensor PIR e comunicação MQTT.

🚀 Sobre o Projeto

O Smart Dispenser foi criado para auxiliar idosos e pessoas com rotina médica a tomarem seus medicamentos nos horários corretos.

Ele:

alerta o usuário nos horários programados

registra que a dose foi tomada

detecta presença com sensor PIR

envia logs via MQTT para cuidadores ou familiares

permite ajuste remoto de horários

possui LEDs, buzzer e servo para liberar o compartimento

Tudo isso com um circuito simples e totalmente simulado no Wokwi.

🛠 Hardware Utilizado

ESP32 DevKit

LCD 16x2 I2C

RTC DS3231

Servo SG90

Buzzer ativo

3 LEDs (verde, amarelo, vermelho)

Sensor PIR

Botão CONFIRMAR

Botão MENU

📡 MQTT – Comunicação em Nuvem

O dispositivo se conecta ao broker:

broker.hivemq.com
porta: 1883

Tópicos enviados:
Tópico	Descrição
medicamentos/status	Remédio pendente
medicamentos/log	Remédio confirmado
medicamentos/alerta	Atraso > 15 min
Tópico recebido:
Tópico	Função
medicamentos/config	Enviar novas configurações

Exemplo:

{"set_hour": 18}

🧠 Funcionamento

O ESP32 lê a hora do RTC.

Quando chega o horário do medicamento:

Acende LED amarelo

Buzzer toca

LCD exibe mensagem

Servo libera o compartimento

Mensagem MQTT enviada

O usuário confirma:

Botão CONFIRMAR

ou sensor PIR (automático)

Se passar 15 minutos:

LED vermelho acende

Mensagem MQTT de atraso enviada

🖥 Simulação no Wokwi

O projeto inclui:

/wokwi/diagram.json


Basta abrir em:

👉 https://wokwi.com/

🔧 Firmware

O firmware está em:

/firmware/SmartDispenser.ino


Inclui:

tratamento de horários

ajuste de hora no LCD

controle do servo

MQTT completo

debug via Serial

leitura do PIR

registro de atraso

📂 Estrutura do Repositório
Smart-Dispenser/
│
├── firmware/
│     └── SmartDispenser.ino
│
├── wokwi/
│     ├── diagram.json
│     ├── libraries.txt
│     └── wokwi-project.txt
│
├── docs/
│     └── artigo.pdf  (opcional)
│
└── README.md

🤝 Contribuições

Pull requests são bem-vindos!
Para sugestões ou melhorias, fique à vontade para abrir uma issue.

📄 Licença

Projeto acadêmico — livre para uso educacional.


