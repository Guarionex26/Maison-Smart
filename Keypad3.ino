// Broche unique analogique utilisée pour tous les boutons
const int buttonPin = A0;

// Seuils de tension pour chaque bouton (à ajuster selon ton montage)
const int keyThresholds[12] = {
  50, 150, 250,   // '1', '2', '3'
  350, 450, 550,  // '4', '5', '6'
  650, 750, 850,  // '7', '8', '9'
  950, 1023, 1024 // '*', '0', '#'
};

// Caractères associés à chaque bouton
const char keyMap[12] = {
  '1', '2', '3',
  '4', '5', '6',
  '7', '8', '9',
  '*', '0', '#'
};

char lastKey = '\0';

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(buttonPin, INPUT);
  Serial.println("Clavier analogique prêt !");
}

void loop() {
  int analogValue = analogRead(buttonPin);

  // Trouver quelle touche correspond à la valeur lue
  for (int i = 0; i < 12; i++) {
    // Ici, on utilise une marge d'erreur de ±20
    if (analogValue >= keyThresholds[i] - 20 && analogValue <= keyThresholds[i] + 20) {
      char key = keyMap[i];

      // Ne pas répéter si même touche maintenue
      if (key != lastKey) {
        Serial.print("Touche pressée : ");
        Serial.println(key);
        lastKey = key;
      }

      delay(200); // Anti-rebond simple
      return;
    }
  }

  // Si aucune touche reconnue
  lastKey = '\0';
}
