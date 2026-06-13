# Build a Complete RFID Smart Cart IoT System

Create a production-ready RFID-based Smart Shopping Cart system consisting of:

## Objective

Build a supermarket smart cart that allows customers to:

1. Create a shopping cart directly from the trolley.
2. Scan products using RFID tags.
3. Automatically add products to a cloud cart.
4. Validate product placement using a load cell.
5. Prevent theft by detecting weight mismatches.
6. Show cart contents in real-time on a customer web page.
7. Generate a UPI QR code at checkout.
8. Calculate the final bill automatically.
9. Manage products through an admin dashboard.

---

# System Architecture

The solution contains 3 major parts.

## Part 1: ESP32 Smart Cart Firmware

Hardware:

* ESP32
* MFRC522 RFID Reader
* HX711 Load Cell Amplifier
* Load Cell
* SSD1306 OLED Display
* Buzzer
* Red LED
* Green LED
* Cart Creation Button
* Checkout Button

Libraries:

* WiFi.h
* HTTPClient.h
* ArduinoJson
* SSD1306
* qrcodeoled
* MFRC522
* HX711

---

## ESP32 Workflow

### Startup Validation

On boot:

* Initialize OLED
* Initialize RFID
* Initialize HX711
* Connect to WiFi
* Display errors if any component fails

Examples:

* RFID FAIL
* LOADCELL FAIL
* WIFI FAIL

---

### Create Cart Button

When pressed:

1. Clear existing cart state.
2. Reset weight tracking.
3. Call:

POST /cart/create

4. Receive cartId.
5. Generate:

[https://smart-cart-174a0.web.app/?cartId=](https://smart-cart-174a0.web.app/?cartId=)<cartId>

6. Show QR code on OLED.

Customer scans QR using mobile.

---

### RFID Scan Flow

When RFID tag detected:

1. Read UID.
2. Convert UID to uppercase hex string.
3. Call:

POST /cart/scan

Request:

```json
{
  "cartId": "abc123",
  "rfid": "A1B2C3D4"
}
```

Response:

```json
{
  "success": true,
  "product": {
    "id": "milk001",
    "name": "Amul Milk",
    "price": 30,
    "weight": 0.5
  }
}
```

Store locally:

* quantity
* price
* expected weight

Show:

Added:
Amul Milk

on OLED.

---

### Weight Validation

After scan:

1. Capture current load cell weight.
2. Wait 3 seconds.
3. Detect weight increase.
4. Compare with product weight.

If valid:

* Green LED
* Buzzer success

If invalid:

* Red LED
* Buzzer error
* Display error

---

### Checkout Flow

Checkout button pressed.

Calculate:

Expected Weight =
sum(productWeight × quantity)

Read actual weight from HX711.

Allow tolerance:

±150 grams

If mismatch:

Display:

Weight Error
Check Items

Do not proceed.

If valid:

Generate total amount.

Build UPI URI:

upi://pay?pa=merchant@ybl&pn=SmartCart&am=<amount>&cu=INR&tr=<cartId>

Display QR code on OLED.

---

### Cart State

Maintain local memory:

```cpp
std::map<String,int> productCount;
std::map<String,float> productPrice;
std::map<String,float> productWeight;
```

Calculate:

* Expected weight
* Total amount

without additional API calls.

---

# Part 2: Backend APIs

Technology:

* Node.js
* Express-style routing
* Firebase Admin SDK
* Firestore

Folder Structure:

```text
functions/
├── index.js
├── routes.js
├── firebase.js
├── handlers/
│   ├── cart.js
│   └── product.js
```

---

## POST /cart/create

Create Firestore cart.

Response:

```json
{
  "success": true,
  "cartId": "generated-id"
}
```

---

## POST /cart/add

Increment quantity.

---

## POST /cart/remove

Decrement quantity.

Delete when quantity reaches zero.

---

## POST /cart/scan

Find product by RFID.

Firestore query:

products
where rfid == scannedRFID

Automatically add product to cart.

Return product details.

---

## GET /product

Return product metadata.

---

# Part 3: Firestore Database

products/{productId}

```json
{
  "name": "Amul Milk",
  "price": 30,
  "weight": 0.5,
  "image": "...",
  "rfid": "A1B2C3D4"
}
```

carts/{cartId}

```json
{
  "createdAt": "timestamp"
}
```

carts/{cartId}/items/{productId}

```json
{
  "productId": "milk001",
  "quantity": 2
}
```

---

# Part 4: Admin Dashboard

Technology:

* HTML
* CSS
* Vanilla JS
* Firestore SDK

Features:

## Product CRUD

Create product.

Fields:

* Product ID
* Name
* Price
* Weight
* Image URL
* RFID Tag

---

## RFID Validation

Before save:

* Query all products
* Ensure RFID unique

Show:

RFID already assigned to another product

if duplicate exists.

---

## Product List

Display:

* Image
* Name
* Price
* Weight
* RFID

Actions:

* Edit
* Delete

---

# Part 5: Customer Cart Website

Technology:

* HTML
* CSS
* JavaScript
* Firestore Realtime

Read:

?cartId=<id>

from URL.

---

## Realtime Updates

Listen using:

onSnapshot()

to:

carts/{cartId}/items

Whenever Firestore changes:

* Reload products
* Recalculate totals
* Update UI

without refreshing page.

---

## Cart UI

Display:

* Product image
* Product name
* Weight
* Price
* Quantity
* Line total

---

## Billing

Calculate:

Total = Σ(price × quantity)

Show live total.

---

# Firebase Hosting

Generate firebase.json.

Admin URL:

/admin

Customer URL:

/

Support SPA rewrites.

---

# Security Requirements

* Validate product existence.
* Validate RFID uniqueness.
* Prevent invalid cart operations.
* Handle API failures gracefully.
* Protect Firestore with security rules.
* Use server timestamps.
* Use atomic increment operations.

---

# UI Requirements

Theme:

Modern retail self-checkout kiosk.

Design:

* Mobile friendly
* Responsive
* Card-based layouts
* Product images
* Clean typography
* Shopping-cart appearance

---

# Deliverables

Generate complete codebase including:

1. ESP32 firmware
2. Firebase project setup
3. Firestore schema
4. Node.js backend
5. REST APIs
6. Admin portal
7. Customer cart portal
8. Realtime synchronization
9. RFID workflow
10. Weight validation system
11. UPI QR payment generation
12. Deployment instructions
13. Firestore security rules
14. Complete folder structure

The solution should compile on ESP32, deploy to Firebase Hosting, connect to Firestore, and run as a complete RFID Smart Cart prototype for supermarkets.
