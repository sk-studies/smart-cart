import { initializeApp } from "https://www.gstatic.com/firebasejs/10.12.0/firebase-app.js";
import {
  getFirestore,
  collection,
  onSnapshot,
  doc,
  getDoc,
  getDocs,
  setDoc,
  deleteDoc,
} from "https://www.gstatic.com/firebasejs/10.12.0/firebase-firestore.js";

// 🔹 Firebase config
const firebaseConfig = {
  apiKey: "AIzaSyApadeBXAXcR5rh4HAnshU7J_ZGpGPeIdw",
  authDomain: "smart-cart-174a0.firebaseapp.com",
  projectId: "smart-cart-174a0",
};

const app = initializeApp(firebaseConfig);
const db = getFirestore(app);

// 🔹 Routing
const path = window.location.pathname;

if (path === "/admin") {
  loadAdminUI();
} else {
  loadCartUI();
}

//////////////////////////////////////////////////////////
// 🛒 CART UI
//////////////////////////////////////////////////////////

function loadCartUI() {
  document.body.innerHTML = `
    <div class="container">
      <h2>🛒 Smart Cart</h2>
      <p id="cartId"></p>
      <div id="cartItems" class="grid"></div>
      <h3 id="total">Total: ₹0</h3>
    </div>
  `;

  const cartId = new URLSearchParams(window.location.search).get("cartId");

  if (!cartId) {
    alert("cartId missing in URL");
    return;
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
      div.querySelector(".plus").onclick = () =>
        addItem(cartId, item.productId);
      div.querySelector(".minus").onclick = () =>
        removeItem(cartId, item.productId);

      div.querySelector(".remove").onclick = async () => {
        await fetch(`https://removeitem-ktoxqz34xq-uc.a.run.app`, {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ cartId, productId: item.productId }),
        });
      };
    }

    document.getElementById("total").innerText = "Total: ₹" + total;
  });
}

// 🔹 API calls
async function addItem(cartId, productId) {
  await fetch(`https://additem-ktoxqz34xq-uc.a.run.app`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ cartId, productId }),
  });
}

async function removeItem(cartId, productId) {
  await fetch(`https://removeitem-ktoxqz34xq-uc.a.run.app`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ cartId, productId }),
  });
}

//////////////////////////////////////////////////////////
// 🛠 ADMIN UI
//////////////////////////////////////////////////////////

function loadAdminUI() {
  document.body.innerHTML = `
    <div class="container">
      <h2>🛠 Admin Panel</h2>

      <form id="productForm">
        <input id="id" placeholder="Product ID" required />
        <input id="name" placeholder="Name" required />
        <input id="price" type="number" placeholder="Price" required />
        <input id="weight" placeholder="Weight" required />
        <input id="image" placeholder="Image URL" required />

        <button type="submit">Save Product</button>
      </form>

      <h3>Products</h3>
      <div id="productList" class="grid"></div>
    </div>
  `;

  handleProductSubmit();
  loadProducts();
}

// 🔹 Create / Update product
function handleProductSubmit() {
  const form = document.getElementById("productForm");

  form.onsubmit = async (e) => {
    e.preventDefault();

    const id = document.getElementById("id").value;

    const product = {
      name: document.getElementById("name").value,
      price: Number(document.getElementById("price").value),
      weight: document.getElementById("weight").value,
      image: document.getElementById("image").value,
    };

    await setDoc(doc(db, "products", id), product);

    alert("Saved");
    form.reset();
    loadProducts();
  };
}

// 🔹 Load products
async function loadProducts() {
  const container = document.getElementById("productList");
  container.innerHTML = "";

  const snapshot = await getDocs(collection(db, "products"));

  snapshot.forEach((docSnap) => {
    const product = docSnap.data();
    const id = docSnap.id;

    const div = document.createElement("div");
    div.className = "card";

    div.innerHTML = `
      <img src="${product.image}" />
      <div>${product.name}</div>
      <div>₹${product.price}</div>
      <div>${product.weight}</div>

      <button class="edit">Edit</button>
      <button class="delete">Delete</button>
    `;

    // 🔹 Edit
    div.querySelector(".edit").onclick = () => {
      document.getElementById("id").value = id;
      document.getElementById("name").value = product.name;
      document.getElementById("price").value = product.price;
      document.getElementById("weight").value = product.weight;
      document.getElementById("image").value = product.image;
    };

    // 🔹 Delete
    div.querySelector(".delete").onclick = async () => {
      await deleteDoc(doc(db, "products", id));
      loadProducts();
    };

    container.appendChild(div);
  });
}
