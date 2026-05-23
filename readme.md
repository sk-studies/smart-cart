# 1. Install Firebase CLI (one-time)

```bash
npm install -g firebase-tools
```

Login:

```bash
firebase login
```

---

# 2. Initialize Functions in your project

Go to your project folder:

```bash
cd your-project
firebase init functions
```

You’ll be asked:

- Select: **Functions**
- Language: **JavaScript** (simpler than TS for now)
- ESLint: optional (you can say No)
- Install dependencies: Yes

This creates:

```text
functions/
  ├── index.js
  ├── package.json
```

---

# 3. Add your functions

Open:

```text
functions/index.js
```

Paste your APIs (example):

```js
const functions = require("firebase-functions");
const admin = require("firebase-admin");

admin.initializeApp();
const db = admin.firestore();

exports.createCart = functions.https.onRequest(async (req, res) => {
  const cartRef = await db.collection("carts").add({
    createdAt: admin.firestore.FieldValue.serverTimestamp(),
  });

  res.json({ cartId: cartRef.id });
});
```

Add others (`addItem`, `removeItem`) the same way.

---

# 4. VERY IMPORTANT (enable billing)

Cloud Functions require Blaze plan.

Go to:

- Firebase Console → Project Settings → Billing
- Upgrade to **Blaze (pay-as-you-go)**

👉 Don’t worry—you’ll likely still pay ₹0

---

# 5. Deploy (this is the “registration” step)

Run:

```bash
firebase deploy --only functions
```

After deploy, you’ll see output like:

```text
Function URL (createCart):
https://us-central1-your-project.cloudfunctions.net/createCart
```

👉 This URL = your live API

---

# 6. Test your function

Open browser or Postman:

```
https://.../createCart
```

You should get:

```json
{
  "cartId": "abc123"
}
```

---

# 7. Region (important but optional)

By default it deploys to `us-central1`.

If you want India/closer latency:

```js
exports.createCart = functions
  .region("asia-south1")
  .https.onRequest(async (req, res) => {
```

👉 This reduces delay for ESP32 calls.

---

# 8. Common mistakes (you’ll likely hit one of these)

### ❌ “Permission denied”

Fix Firestore rules temporarily:

```js
rules_version = '2';
service cloud.firestore {
  match /databases/{database}/documents {
    match /{document=**} {
      allow read, write: if true;
    }
  }
}
```

(Only for testing—don’t keep this forever)

---

### ❌ CORS error (for browser, not ESP)

Add `cors` package

---

### ❌ Function not updating

Run:

```bash
firebase deploy --only functions
```

again (it doesn’t auto-update)

---

# 9. How ESP32 will use it

Once deployed, your ESP calls:

```cpp
http.begin("https://your-url/createCart");
```

That’s it—no Firebase SDK needed on ESP.

---

# 10. What “registering functions” really means

You’re basically:

- Writing Node.js code
- Uploading it to Google servers
- Getting a public HTTPS endpoint

👉 That endpoint = your backend API

---

# Final thought

Once you deploy successfully once, everything becomes easy:

- Modify code → deploy again
- ESP keeps calling same URLs

---

If you want, next I can help you:

- Debug your first deploy (if it fails)
- Add logging (`console.log`) and view logs
- Secure your API so others can’t misuse it

Just tell me where you're stuck.

to deploy:
firebase deploy --only functions
