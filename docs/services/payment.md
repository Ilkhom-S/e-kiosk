# Payment Service

The Payment Service manages payment creation, processing, and transaction handling.

## Overview

The Payment Service (`IPaymentService`) provides:

- Payment creation and lifecycle management
- Payment provider information retrieval
- Payment field/parameter management
- Payment search and filtering
- Payment processing and conversion
- Payment cancellation and cleanup

## Core Operations

### Create Payment

```cpp
auto paymentService = core->getPaymentService();

// Create new payment for operator
qint64 paymentId = paymentService->createPayment(operatorId);
```

### Manage Active Payment

```cpp
// Get active payment
qint64 activeId = paymentService->getActivePayment();

// Deactivate current payment
paymentService->deactivatePayment();
```

### Payment Processing

```cpp
// Process payment (online or offline)
bool processed = paymentService->processPayment(paymentId, true); // true = online

// Convert payment type
bool converted = paymentService->convertPayment(paymentId, "newType");

// Cancel payment
bool cancelled = paymentService->cancelPayment(paymentId);
```

### Payment Fields

```cpp
// Get payment field
IPayment::SParameter field = paymentService->getPaymentField(paymentId, "amount");

// Get all fields for payment
QList<IPayment::SParameter> fields = paymentService->getPaymentFields(paymentId);

// Update payment fields
paymentService->updatePaymentField(paymentId, "amount", value);
```

### Payment Search

```cpp
// Find payments by date and phone
QList<qint64> payments = paymentService->findPayments(QDate::currentDate(), "+1234567890");
```

## Limitations

- Actual interface (32 methods) partially documented
- See IPaymentService.h for complete API

## File Reference

- Implementation: [IPaymentService.h](../../include/SDK/PaymentProcessor/Core/IPaymentService.h)
