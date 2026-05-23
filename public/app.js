import { initializeApp } from "https://www.gstatic.com/firebasejs/10.12.0/firebase-app.js";
import {
  getFirestore,
  collection,
  onSnapshot,
} from "https://www.gstatic.com/firebasejs/10.12.0/firebase-firestore.js";

// 🔹 Your Firebase config
// const firebaseConfig = {
//   apiKey: "YOUR_API_KEY",
//   authDomain: "YOUR_PROJECT.firebaseapp.com",
//   projectId: "YOUR_PROJECT_ID",
// };
const firebaseConfig = {
  apiKey: "AIzaSyApadeBXAXcR5rh4HAnshU7J_ZGpGPeIdw",
  authDomain: "smart-cart-174a0.firebaseapp.com",
  projectId: "smart-cart-174a0",
  storageBucket: "smart-cart-174a0.firebasestorage.app",
  messagingSenderId: "335149368283",
  appId: "1:335149368283:web:099ee851b647c647c01dfe",
};

const app = initializeApp(firebaseConfig);
const db = getFirestore(app);

// 🔹 Get cartId from URL
const urlParams = new URLSearchParams(window.location.search);
const cartId = urlParams.get("cartId");

document.getElementById("cartId").innerText = "Cart ID: " + cartId;

if (!cartId) {
  alert("No cartId provided in URL");
}

// 🔹 Listen to cart items (REAL-TIME)
const cartRef = collection(db, "carts", cartId, "items");

onSnapshot(cartRef, (snapshot) => {
  const list = document.getElementById("cartItems");
  list.innerHTML = "";

  snapshot.forEach((doc) => {
    const item = doc.data();

    const li = document.createElement("li");
    li.innerText = `${item.productId} → Qty: ${item.quantity}`;

    list.appendChild(li);
  });
});
