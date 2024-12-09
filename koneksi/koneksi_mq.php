<?php
// Koneksi ke database
$host = "localhost";
$user = "root";          // Ganti dengan username database Anda
$pass = "";              // Ganti dengan password database Anda
$db = "sensor_data";     // Nama database

$conn = new mysqli($host, $user, $pass, $db);

// Cek koneksi
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Ambil data dari request POST dan validasi
$sensor_value = isset($_POST['mq137_value']) ? $_POST['mq137_value'] : null;
$voltage = isset($_POST['mq137_voltage']) ? $_POST['mq137_voltage'] : null;
$ppm = isset($_POST['nh3_ppm']) ? $_POST['nh3_ppm'] : null;

if ($sensor_value === null || $voltage === null || $ppm === null) {
    die("Error: Missing required POST data");
}

// Query untuk menyimpan data (menggunakan prepared statement)
$stmt = $conn->prepare("INSERT INTO mq137_readings (analog_value, voltage, ppm) VALUES (?, ?, ?)");
$stmt->bind_param("idd", $sensor_value, $voltage, $ppm);

if ($stmt->execute()) {
    echo "Data berhasil disimpan";
} else {
    echo "Error: " . $stmt->error;
}

// Tutup koneksi
$stmt->close();
$conn->close();
?>
