import { initializeApp } from "https://www.gstatic.com/firebasejs/10.12.0/firebase-app.js";
import {
  getFirestore,
  collection,
  onSnapshot,
  doc,
  getDoc,
} from "https://www.gstatic.com/firebasejs/10.12.0/firebase-firestore.js";

const firebaseConfig = {
  apiKey: "AIzaSyApadeBXAXcR5rh4HAnshU7J_ZGpGPeIdw",
  authDomain: "smart-cart-174a0.firebaseapp.com",
  projectId: "smart-cart-174a0",
};

const app = initializeApp(firebaseConfig);
const db = getFirestore(app);

// cartId from URL
const cartId = new URLSearchParams(window.location.search).get("cartId");

document.getElementById("cartId").innerText = "Cart ID: " + cartId;

const cartRef = collection(db, "carts", cartId, "items");

onSnapshot(cartRef, async (snapshot) => {
  const container = document.getElementById("cartItems");
  container.innerHTML = "";

  let total = 0;

  for (const d of snapshot.docs) {
    const item = d.data();

    // 🔹 fetch product details
    const productDoc = await getDoc(doc(db, "products", item.productId));
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
      <div class="qty">Qty: ${item.quantity}</div>
      <div>Total: ₹${itemTotal}</div>
    `;

    container.appendChild(div);
  }

  document.getElementById("total").innerText = "Total: ₹" + total;
});
