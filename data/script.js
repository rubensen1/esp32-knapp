document.addEventListener('DOMContentLoaded', () => {
    loadQuestions();

    const ctx = document.getElementById('pollChart').getContext('2d');
    const chartTypeSelect = document.getElementById('chartType');
    const promptLabel = document.getElementById('promptLabel');

    // Set the prompt label
    promptLabel.textContent = 'Prompt: JA, NEI, KANSKJE';

    let pollChart;

    function createChart(type) {
      if (pollChart) {
        pollChart.destroy();
      }
      pollChart = new Chart(ctx, {
        type: type,
        data: {
          labels: ['JA', 'KANSKJE', 'NEI'],
          datasets: [{
            label: 'Poll Results',
            data: [10, 5, 3], // Replace with real data
            backgroundColor: [
              'rgba(2, 213, 57, 0.8)',
              'rgba(248, 209, 0, 0.8)',
              'rgba(218, 0, 0, 0.8)'
            ],
            borderWidth: 1
          }]
        },
        options: {
          responsive: true,
          plugins: {
            legend: {
              position: 'top'
            }
          }
        }
      });
    }

    // Initialize with default chart type
    createChart(chartTypeSelect.value);

    // Add event listener for dropdown
    chartTypeSelect.addEventListener('change', () => {
    console.log('Selected chart type:', chartTypeSelect.value); // To verify
    createChart(chartTypeSelect.value);
  });
});

function sendQuestion() {
  const input = document.getElementById('question');
  const text = input.value.trim();
  if (text === "") return;

  // Legg til spørsmålet i visningen
  addQuestionToList(text);

  // Lagre spørsmålet i nettleseren
  saveQuestion(text);

  // Tøm inputfeltet
  input.value = '';
}

// ✅ Funksjon som legger til et spørsmål i listen på skjermen
function addQuestionToList(text) {
  const container = document.createElement('div');
  container.classList.add('question-item');

  const checkbox = document.createElement('input');
  checkbox.type = 'checkbox';
  checkbox.id = text;

  const label = document.createElement('label');
  label.setAttribute('for', text);
  label.textContent = text;

  container.appendChild(checkbox);
  container.appendChild(label);

  document.getElementById('questionList').appendChild(container);
}

// 💾 Lagre spørsmål i localStorage
function saveQuestion(text) {
  let questions = JSON.parse(localStorage.getItem('questions')) || [];
  questions.push(text);
  localStorage.setItem('questions', JSON.stringify(questions));
}

// 📦 Last spørsmål fra localStorage ved oppstart
function loadQuestions() {
  let questions = JSON.parse(localStorage.getItem('questions')) || [];
  questions.forEach(q => addQuestionToList(q));
}

//slett alle spørsmål
function clearQuestions() {
  localStorage.removeItem('questions');
  document.getElementById('questionList').innerHTML = '';
}

// Samler markerte spørsmål og sender som JSON
function sendToScreen() {
  const checked = [];
  document.querySelectorAll('#questionList input[type="checkbox"]:checked')
    .forEach(cb => checked.push(cb.id));

  if (checked.length === 0) {
    alert("Please select at least one question.");
    return;
  }

  fetch('/api/screen', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ questions: checked })
  })
  .then(response => {
    if (!response.ok) throw new Error("Failed to send data");
    alert("Sent to screen!");
  })
  .catch(err => {
    console.error(err);
    alert("Error sending data to ESP32.");
  });
  }

// Functions for toggling options
function setLys(option) {
  toggleOption('lys-buttons', option);
  lysState = option;
  console.log('Lys:', lysState);
}

function setLyd(option) {
  toggleOption('lyd-buttons', option);
  lydState = option;
  console.log('Lyd:', lydState);
}

function setBlindmodus(option) {
  toggleOption('blind-buttons', option);
  blindState = option;
  console.log('Blindmodus:', blindState);
}

function toggleOption(containerId, selectedOption) {
  const container = document.getElementById(containerId);
  const buttons = container.querySelectorAll('.option-btn');

  buttons.forEach(btn => {
    if (btn.textContent === selectedOption) {
      btn.classList.add('selected');
    } else {
      btn.classList.remove('selected');
    }
  });
}

// Initialize default states after DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
  // Set default button states
  document.querySelector('#lys-buttons .option-btn:nth-child(2)').classList.add('selected');
  document.querySelector('#lyd-buttons .option-btn:nth-child(2)').classList.add('selected');
 
  // Set default for Blindmodus to 'AV'
  setBlindmodus('AV');

  console.log('Initial states:', { lysState, lydState, blindState });
  // Your other initialization code...
});
