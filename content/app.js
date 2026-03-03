const HISTORY_API_URL = "/api/history.json";
const REFRESH_INTERVAL_MS = 5000;

let chart;

function formatElapsed(ms) {
  const totalSeconds = Math.floor(ms / 1000);
  const minutes = Math.floor(totalSeconds / 60);
  const seconds = totalSeconds % 60;
  return `${minutes}:${String(seconds).padStart(2, "0")}`;
}

async function fetchHistory() {
  const response = await fetch(HISTORY_API_URL, { cache: "no-store" });
  if (!response.ok) {
    throw new Error(`history fetch failed: ${response.status}`);
  }
  return response.json();
}

function ensureChart() {
  if (chart) {
    return chart;
  }

  const ctx = document.getElementById("sensorChart").getContext("2d");
  chart = new Chart(ctx, {
    type: "line",
    data: {
      labels: [],
      datasets: [
        {
          label: "温度 (°C)",
          data: [],
          borderColor: "rgb(255, 99, 132)",
          yAxisID: "y",
          tension: 0.2,
          pointRadius: 0,
        },
        {
          label: "湿度 (%)",
          data: [],
          borderColor: "rgb(54, 162, 235)",
          yAxisID: "y1",
          tension: 0.2,
          pointRadius: 0,
        },
      ],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        y: {
          type: "linear",
          display: true,
          position: "left",
          title: { display: true, text: "温度 (°C)" },
        },
        y1: {
          type: "linear",
          display: true,
          position: "right",
          title: { display: true, text: "湿度 (%)" },
          grid: { drawOnChartArea: false },
        },
      },
      plugins: {
        legend: { display: true },
      },
    },
  });

  return chart;
}

function applyHistory(payload) {
  const points = Array.isArray(payload?.points) ? payload.points : [];
  const labels = points.map((point) => formatElapsed(point[0]));
  const temperatures = points.map((point) => point[1]);
  const humidities = points.map((point) => point[2]);

  const currentChart = ensureChart();
  currentChart.data.labels = labels;
  currentChart.data.datasets[0].data = temperatures;
  currentChart.data.datasets[1].data = humidities;
  currentChart.update();
}

async function refreshChart() {
  try {
    const payload = await fetchHistory();
    applyHistory(payload);
  } catch (error) {
    console.error(error);
  }
}

refreshChart();
setInterval(refreshChart, REFRESH_INTERVAL_MS);
