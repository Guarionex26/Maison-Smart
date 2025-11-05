// === Définition des broches des boutons ===
// 12 boutons connectés à des broches séparées
const int buttonPins[12] = {2, 5, 9, 12, 8, 7, 13, 3, 6,  11, 4, 10 };

// Les caractères associés à chaque bouton
const char keyMap[12] = {
  '1', '2', '3',
  '4', '5', '6',
  '7', '8', '9',
  '*', '0', '#'
};

// État précédent de chaque bouton
bool buttonState[12] = { false };

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialiser les broches des boutons
  for (int i = 0; i < 12; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);  // Active résistance pull-up interne
  }

  Serial.println("Clavier prêt !");
}

void loop() {
  for (int i = 0; i < 12; i++) {
    bool pressed = digitalRead(buttonPins[i]) == LOW; // LOW = bouton appuyé

    if (pressed && !buttonState[i]) {
      char key = keyMap[i];

      Serial.print("Touche pressée : ");
      Serial.println(key);

      buttonState[i] = true;
    }

    if (!pressed && buttonState[i]) {
      buttonState[i] = false;
    }
  }

  delay(10); // Anti-rebond
}
