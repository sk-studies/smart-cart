const { setGlobalOptions } = require("firebase-functions");
const { onRequest } = require("firebase-functions/https");
const logger = require("firebase-functions/logger");
const admin = require("firebase-admin");

setGlobalOptions({ maxInstances: 10 });

admin.initializeApp();
const db = admin.firestore();

// 🔹 1. CREATE CART
exports.createCart = onRequest(async (req, res) => {
  // 🔹 CORS headers
  res.set("Access-Control-Allow-Origin", "*");
  res.set("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  res.set("Access-Control-Allow-Headers", "Content-Type");

  // 🔹 Handle preflight request
  if (req.method === "OPTIONS") {
    return res.status(204).send("");
  }

  try {
    const cartRef = await db.collection("carts").add({
      createdAt: admin.firestore.FieldValue.serverTimestamp(),
    });

    logger.info("Cart created:", cartRef.id);

    res.status(200).json({
      success: true,
      cartId: cartRef.id,
    });
  } catch (error) {
    logger.error(error);
    res.status(500).json({ success: false, error: error.message });
  }
});

// 🔹 2. ADD ITEM TO CART
exports.addItem = onRequest(async (req, res) => {
  // 🔹 CORS headers
  res.set("Access-Control-Allow-Origin", "*");
  res.set("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  res.set("Access-Control-Allow-Headers", "Content-Type");

  // 🔹 Handle preflight request
  if (req.method === "OPTIONS") {
    return res.status(204).send("");
  }

  try {
    const { cartId, productId } = req.body;

    if (!cartId || !productId) {
      return res.status(400).json({
        success: false,
        message: "cartId and productId required",
      });
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
      const data = doc.data();

      await itemRef.set({
        ...data,
        quantity: data.quantity + 1,
        updatedAt: admin.firestore.FieldValue.serverTimestamp(),
      });
    } else {
      await itemRef.set({
        productId: productId,
        quantity: 1,
        createdAt: admin.firestore.FieldValue.serverTimestamp(),
      });
    }

    logger.info(`Item added: ${productId} → Cart: ${cartId}`);

    res.status(200).json({
      success: true,
      message: "Item added",
    });
  } catch (error) {
    logger.error(error);
    res.status(500).json({ success: false, error: error.message });
  }
});

// 🔹 3. REMOVE ITEM FROM CART
exports.removeItem = onRequest(async (req, res) => {
  // 🔹 CORS headers
  res.set("Access-Control-Allow-Origin", "*");
  res.set("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  res.set("Access-Control-Allow-Headers", "Content-Type");

  // 🔹 Handle preflight request
  if (req.method === "OPTIONS") {
    return res.status(204).send("");
  }

  try {
    const { cartId, productId } = req.body;

    if (!cartId || !productId) {
      return res.status(400).json({
        success: false,
        message: "cartId and productId required",
      });
    }

    const itemRef = db
      .collection("carts")
      .doc(cartId)
      .collection("items")
      .doc(productId);

    const doc = await itemRef.get();

    if (!doc.exists) {
      return res.status(404).json({
        success: false,
        message: "Item not found",
      });
    }

    const data = doc.data();

    if (data.quantity > 1) {
      await itemRef.set({
        ...data,
        quantity: data.quantity - 1,
        updatedAt: admin.firestore.FieldValue.serverTimestamp(),
      });
    } else {
      await itemRef.delete();
    }

    logger.info(`Item removed: ${productId} → Cart: ${cartId}`);

    res.status(200).json({
      success: true,
      message: "Item removed",
    });
  } catch (error) {
    logger.error(error);
    res.status(500).json({ success: false, error: error.message });
  }
});

exports.getProduct = onRequest(async (req, res) => {
  try {
    const productId = req.query.productId;

    if (!productId) {
      return res.status(400).json({ error: "productId required" });
    }

    const doc = await db.collection("products").doc(productId).get();

    if (!doc.exists) {
      return res.status(404).json({ error: "Product not found" });
    }

    res.json(doc.data());
  } catch (e) {
    res.status(500).json({ error: e.message });
  }
});
