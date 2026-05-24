const admin = require("firebase-admin");
const db = admin.firestore();

exports.createCart = async (req, res) => {
  const cartRef = await db.collection("carts").add({
    createdAt: admin.firestore.FieldValue.serverTimestamp(),
  });

  res.json({ success: true, cartId: cartRef.id });
};

exports.addItem = async (req, res) => {
  const { cartId, productId } = req.body;

  if (!cartId || !productId) {
    return res.status(400).json({ error: "cartId & productId required" });
  }

  const productDoc = await db.collection("products").doc(productId).get();
  if (!productDoc.exists) {
    return res.status(404).json({ error: "Invalid product" });
  }

  const itemRef = db
    .collection("carts")
    .doc(cartId)
    .collection("items")
    .doc(productId);

  const doc = await itemRef.get();

  if (doc.exists) {
    await itemRef.update({
      quantity: admin.firestore.FieldValue.increment(1),
      updatedAt: admin.firestore.FieldValue.serverTimestamp(),
    });
  } else {
    await itemRef.set({
      productId,
      quantity: 1,
      createdAt: admin.firestore.FieldValue.serverTimestamp(),
    });
  }

  res.json({ success: true });
};

exports.removeItem = async (req, res) => {
  const { cartId, productId } = req.body;

  const itemRef = db
    .collection("carts")
    .doc(cartId)
    .collection("items")
    .doc(productId);

  const doc = await itemRef.get();

  if (!doc.exists) {
    return res.status(404).json({ error: "Item not found" });
  }

  const data = doc.data();

  if (data.quantity > 1) {
    await itemRef.update({
      quantity: admin.firestore.FieldValue.increment(-1),
      updatedAt: admin.firestore.FieldValue.serverTimestamp(),
    });
  } else {
    await itemRef.delete();
  }

  res.json({ success: true });
};

exports.scanRfid = async (req, res) => {
  const { cartId, rfid } = req.body;

  if (!cartId || !rfid) {
    return res.status(400).json({ error: "cartId & rfid required" });
  }

  // 🔹 Find product by RFID
  const snapshot = await db
    .collection("products")
    .where("rfid", "==", rfid)
    .limit(1)
    .get();

  if (snapshot.empty) {
    return res.status(404).json({ error: "Unknown RFID" });
  }

  const productDoc = snapshot.docs[0];
  const productId = productDoc.id;

  // 🔹 Reuse existing logic
  const itemRef = db
    .collection("carts")
    .doc(cartId)
    .collection("items")
    .doc(productId);

  const doc = await itemRef.get();

  if (doc.exists) {
    await itemRef.update({
      quantity: admin.firestore.FieldValue.increment(1),
      updatedAt: admin.firestore.FieldValue.serverTimestamp(),
    });
  } else {
    await itemRef.set({
      productId,
      quantity: 1,
      createdAt: admin.firestore.FieldValue.serverTimestamp(),
    });
  }

  res.json({ success: true, productId });
};
