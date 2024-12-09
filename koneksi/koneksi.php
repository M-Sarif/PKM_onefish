<?php
// Konfigurasi koneksi database
$servername = "localhost";
$username = "u754565618_senpai";
$password = "Novalsense1";
$dbname = "u754565618_senpai";

// Buat koneksi ke database
$conn = new mysqli($servername, $username, $password, $dbname);

// Periksa koneksi
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Inisialisasi variabel untuk menyimpan nilai POST dari sensor pH
    $temperature = isset($_POST['temperature']) ? $_POST['temperature'] : null;
    $ph = isset($_POST['ph']) ? $_POST['ph'] : null;

    // Inisialisasi variabel untuk menyimpan nilai POST dari sensor MQ137
    $analog_value = isset($_POST['analogValue']) ? $_POST['analogValue'] : null;
    $voltage = isset($_POST['voltage']) ? $_POST['voltage'] : null;
    $resistance = isset($_POST['resistance']) ? $_POST['resistance'] : null;
    $ppm = isset($_POST['ppm']) ? $_POST['ppm'] : null;

    // Cek apakah data pH tersedia
    if ($temperature !== null && $ph !== null) {
        $sql_ph = "INSERT INTO ph_data (temperature, ph) VALUES (?, ?)";
        $stmt_ph = $conn->prepare($sql_ph);
        $stmt_ph->bind_param("dd", $temperature, $ph);
        
        if ($stmt_ph->execute()) {
            echo "New pH record created successfully\n";
        } else {
            echo "Error: " . $stmt_ph->error . "\n";
        }
        $stmt_ph->close();
    }

    // Cek apakah data MQ137 tersedia
    if ($analog_value !== null && $voltage !== null && $resistance !== null && $ppm !== null) {
        $sql_mq137 = "INSERT INTO mq137_readings (analog_value, voltage, resistance, ppm) VALUES (?, ?, ?, ?)";
        $stmt_mq137 = $conn->prepare($sql_mq137);
        $stmt_mq137->bind_param("iddd", $analog_value, $voltage, $resistance, $ppm);
        
        if ($stmt_mq137->execute()) {
            echo "New MQ137 record created successfully\n";
        } else {
            echo "Error: " . $stmt_mq137->error . "\n";
        }
        $stmt_mq137->close();
    }
}

$conn->close();
?>