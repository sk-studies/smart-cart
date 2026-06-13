const cart = require("./handlers/cart");
const product = require("./handlers/product");

const routes = [
  {
    path: "/cart/create",
    method: "POST",
    handler: cart.createCart,
  },
  {
    path: "/cart/add",
    method: "POST",
    handler: cart.addItem,
  },
  {
    path: "/cart/remove",
    method: "POST",
    handler: cart.removeItem,
  },
  {
    path: "/cart/scan",
    method: "POST",
    handler: cart.scanRfid,
  },
  {
    path: "/product",
    method: "GET",
    handler: product.getProduct,
  },
];

module.exports = routes;
