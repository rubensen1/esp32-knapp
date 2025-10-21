document.addEventListener('DOMContentLoaded', loadQuestions);

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


    // Sende spørsmål til ESP -> LCD
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


document.addEventListener('DOMContentLoaded', () => {
  const btn = document.getElementById('toggleBtn');
  const qstList = document.getElementById('ledStatus');

  // Hent status ved lasting
  fetch('/api/status')
    .then(res => res.json())
    .then(data => {
      statusEl.textContent = data.led ? 'LED er PÅ' : 'LED er AV';
    });

  btn.addEventListener('click', () => {
    fetch('/api/toggle', { method: 'POST' })
      .then(res => res.json())
      .then(data => {
        statusEl.textContent = data.led ? 'LED er PÅ' : 'LED er AV';
      });
  });
});