function getRandomValue(min, max, decimals = 0) {
    return (Math.random() * (max - min) + min).toFixed(decimals);
}

// --- KONFIGURASI CHART (MINIMALIS) ---
const commonOptions = {
    responsive: true,
    maintainAspectRatio: false, // Wajib false agar ikut container
    plugins: { 
        legend: { display: false }, // Hilangkan legenda
        tooltip: { enabled: true }
    }, 
    scales: {
        x: { 
            display: false, // Hilangkan sumbu X
            grid: { display: false }
        }, 
        y: { 
            display: true, // Tampilkan angka Y tapi kecil
            ticks: { font: { size: 9 } }, // Font kecil
            grid: { color: '#f0f0f0' } // Garis tipis
        }
    },
    elements: { 
        point: { radius: 0, hitRadius: 10 }, // Titik hilang, tapi bisa dihover
        line: { borderWidth: 2 } 
    },
    layout: { padding: 0 }
};

// --- CHART VOLT ---
const ctxVolt = document.getElementById('chartVolt').getContext('2d');
const chartVolt = new Chart(ctxVolt, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Volt',
            data: [],
            borderColor: '#eab308',
            backgroundColor: 'rgba(234, 179, 8, 0.1)',
            fill: true,
            tension: 0.4
        }]
    },
    options: commonOptions
});

// --- CHART AMPERE ---
const ctxAmp = document.getElementById('chartAmpere').getContext('2d');
const chartAmpere = new Chart(ctxAmp, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Ampere',
            data: [],
            borderColor: '#22c55e',
            backgroundColor: 'rgba(34, 197, 94, 0.1)',
            fill: true,
            tension: 0.4
        }]
    },
    options: commonOptions
});

// --- UPDATE LOOP ---
function updateDashboard() {
    const sampah = getRandomValue(0, 100, 0); 
    const air = getRandomValue(10, 50, 0); 
    let volt = parseFloat(getRandomValue(11.8, 12.8, 2));
    let ampere = parseFloat(getRandomValue(0.5, 2.5, 2));
    let watt = (volt * ampere).toFixed(1);

    // Update DOM
    document.getElementById('sampah-val').innerText = sampah;
    const bar = document.getElementById('sampah-bar');
    bar.style.width = sampah + '%';
    bar.style.backgroundColor = sampah > 80 ? '#ef4444' : '#2ecc71';

    document.getElementById('air-val').innerText = air;
    document.getElementById('volt-val').innerText = volt.toFixed(2);
    document.getElementById('ampere-val').innerText = ampere.toFixed(2);
    document.getElementById('watt-val').innerText = watt;

    // Update Chart
    const now = new Date().toLocaleTimeString();
    
    // Volt
    chartVolt.data.labels.push(now);
    chartVolt.data.datasets[0].data.push(volt);
    if(chartVolt.data.labels.length > 20) {
        chartVolt.data.labels.shift();
        chartVolt.data.datasets[0].data.shift();
    }
    chartVolt.update();

    // Ampere
    chartAmpere.data.labels.push(now);
    chartAmpere.data.datasets[0].data.push(ampere);
    if(chartAmpere.data.labels.length > 20) {
        chartAmpere.data.labels.shift();
        chartAmpere.data.datasets[0].data.shift();
    }
    chartAmpere.update();
}

setInterval(updateDashboard, 1500);
updateDashboard();