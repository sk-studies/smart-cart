const admin = require("firebase-admin");
const db = admin.firestore();

exports.getProduct = async (req, res) => {
  const productId = req.query.productId;

  if (!productId) {
    return res.status(400).json({ error: "productId required" });
  }

  const doc = await db.collection("products").doc(productId).get();

  if (!doc.exists) {
    return res.status(404).json({ error: "Product not found" });
  }

  res.json(doc.data());
};
