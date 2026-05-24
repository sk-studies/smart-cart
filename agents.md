BASE_URL = "https://api-ktoxqz34xq-el.a.run.app";

esp32 module.
reest button tap => will create new cart show the qr code in the oled display.
to create cart post requets on BASE_URL + "/cart/create" with "{}" empty payload.
will return response like
{
"success": true,
"cartId": "d1ELV1zItIOYNFmYARiq"
}
store cartId. and show it in oled display as qr: "https://smart-cart-174a0.web.app/?cartId=1233"

now check if rfid is scanned then
after scaning rfid code post BASEURL + /cart/scan wih rfid and cartId as post params.
will return product with weight field.
if scanned multiple time multiply weight with times and check weight is correct or not. if not show red led

on click of start button reset everything.
