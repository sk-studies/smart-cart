import { initializeApp } from "https://www.gstatic.com/firebasejs/10.12.0/firebase-app.js";
import {
  getFirestore,
  collection,
  onSnapshot,
  doc,
  getDoc,
} from "https://www.gstatic.com/firebasejs/10.12.0/firebase-firestore.js";

const BASE_URL = "https://api-ktoxqz34xq-el.a.run.app";

// 🔹 Firebase config
const firebaseConfig = {
  apiKey: "AIzaSyApadeBXAXcR5rh4HAnshU7J_ZGpGPeIdw",
  authDomain: "smart-cart-174a0.firebaseapp.com",
  projectId: "smart-cart-174a0",
};

const app = initializeApp(firebaseConfig);
const db = getFirestore(app);

const cartId = new URLSearchParams(window.location.search).get("cartId");

if (!cartId) {
  alert("cartId missing in URL");
  throw new Error("Invalid cart url");
}

document.getElementById("cartId").innerText = "Cart ID: " + cartId;

const cartRef = collection(db, "carts", cartId, "items");

onSnapshot(cartRef, async (snapshot) => {
  const container = document.getElementById("cartItems");
  container.innerHTML = "";

  let total = 0;

  for (const d of snapshot.docs) {
    const item = d.data();

    const productDoc = await getDoc(doc(db, "products", item.productId));
    if (!productDoc.exists()) continue;

    const product = productDoc.data();

    const itemTotal = product.price * item.quantity;
    total += itemTotal;

    const div = document.createElement("div");
    div.className = "card";

    div.innerHTML = `
        <img src="${product.image}" />
        <div class="name">${product.name}</div>
        <div>${product.weight}</div>
        <div class="price">₹${product.price}</div>

        <div class="qty-controls">
          <button class="btn minus">-</button>
          <span>${item.quantity}</span>
          <button class="btn plus">+</button>
        </div>

        <div>Total: ₹${itemTotal}</div>

        <button class="remove">Remove</button>
      `;

    container.appendChild(div);

    // 🔹 Events
    div.querySelector(".plus").onclick = () => addItem(cartId, item.productId);
    div.querySelector(".minus").onclick = () =>
      removeItem(cartId, item.productId);

    div.querySelector(".remove").onclick = async () => {
      await fetch(`${BASE_URL}/cart/remove`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ cartId, productId: item.productId }),
      });
    };
  }

  document.getElementById("total").innerText = "Total: ₹" + total;
});

async function addItem(cartId, productId) {
  await fetch(`${BASE_URL}/cart/add`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ cartId, productId }),
  });
}

async function removeItem(cartId, productId) {
  await fetch(`${BASE_URL}/cart/remove`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ cartId, productId }),
  });
}
