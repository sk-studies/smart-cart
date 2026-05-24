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

const form = document.getElementById("productForm");

form.onsubmit = async (e) => {
  e.preventDefault();

  const id = document.getElementById("id").value;

  const product = {
    name: document.getElementById("name").value,
    price: Number(document.getElementById("price").value),
    weight: document.getElementById("weight").value,
    image: document.getElementById("image").value,
    rfid: document.getElementById("rfid").value.trim(),
  };

  const rfid = product.rfid;

  const snapshot = await getDocs(collection(db, "products"));

  const duplicate = snapshot.docs.find(
    (d) => d.data().rfid === rfid && d.id !== id,
  );

  if (duplicate) {
    alert("RFID already assigned to another product");
    return;
  }

  await setDoc(doc(db, "products", id), product);

  alert("Saved");
  form.reset();
  loadProducts();
};

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
      <div class="rfid">RFID: ${product.rfid || "-"}</div>

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
      document.getElementById("rfid").value = product.rfid || "";
    };

    // 🔹 Delete
    div.querySelector(".delete").onclick = async () => {
      await deleteDoc(doc(db, "products", id));
      loadProducts();
    };

    container.appendChild(div);
  });
}

loadProducts();
