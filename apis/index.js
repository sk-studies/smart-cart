const { onRequest } = require("firebase-functions/https");
const { setGlobalOptions } = require("firebase-functions");
const admin = require("firebase-admin");

setGlobalOptions({ maxInstances: 10 });
admin.initializeApp();

const routes = require("./routes");

exports.api = onRequest(
  {
    region: "asia-south1",
  },
  async (req, res) => {
    // 🔹 Global CORS
    res.set("Access-Control-Allow-Origin", "*");
    res.set("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set("Access-Control-Allow-Headers", "Content-Type");

    if (req.method === "OPTIONS") {
      return res.status(204).send("");
    }

    try {
      const method = req.method;
      const path = req.path.replace(/\/$/, "");

      // 🔹 Find matching route
      const route = routes.find((r) => r.path === path && r.method === method);

      if (!route) {
        return res.status(404).json({ error: "Route not found" });
      }

      // 🔹 Execute handler
      return await route.handler(req, res);
    } catch (e) {
      res.status(500).json({ error: e.message });
    }
  },
);
