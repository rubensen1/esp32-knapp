document.addEventListener('DOMContentLoaded', () => {
  // Last eksisterende spørsmål ved oppstart
  loadQuestions();
});

function sendQuestion() {
  const input = document.getElementById('question');
  const text = input.value.trim();
  if (text === "") return;

  // Lagre spørsmålet på ESP32 (SPIFFS)
  saveQuestion(text);

  // Tøm inputfeltet
  input.value = '';
}

// ✅ Funksjon som legger til et spørsmål i listen på skjermen
function addQuestionToList(id, text) {
  const container = document.createElement('div');
  container.classList.add('question-item');

  const checkbox = document.createElement('input');
  checkbox.type = 'checkbox';
  checkbox.id = 'q-' + id;
  checkbox.dataset.questionId = id;
  checkbox.dataset.questionText = text;

  const label = document.createElement('label');
  label.setAttribute('for', 'q-' + id);
  label.textContent = text;

  container.appendChild(checkbox);
  container.appendChild(label);

  document.getElementById('questionList').appendChild(container);
}

// 💾 Lagre spørsmål i SPIFFS via ESP32
function saveQuestion(text) {
  fetch('/api/questions', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ question: text })
  })
  .then(response => {
    if (!response.ok) throw new Error("Failed to save question");
    return response.json();
  })
  .then(data => {
    if (data.success && data.id !== undefined && data.text) {
      // Legg til spørsmålet i visningen etter vellykket lagring
      addQuestionToList(data.id, data.text);
      console.log('Question saved with ID:', data.id);
    } else {
      console.error('Invalid response from server:', data);
    }
  })
  .catch(err => {
    console.error('Error saving question:', err);
    alert("Error saving question to ESP32.");
  });
}

// 📦 Last spørsmål fra SPIFFS via ESP32 ved oppstart
function loadQuestions() {
  fetch('/api/questions', {
    method: 'GET',
    headers: { 'Content-Type': 'application/json' }
  })
  .then(response => {
    if (!response.ok) throw new Error("Failed to load questions");
    return response.json();
  })
  .then(questions => {
    // Tøm listen først
    document.getElementById('questionList').innerHTML = '';
    
    console.log('Loaded questions:', questions);
    
    // Legg til alle spørsmål i visningen
    if (Array.isArray(questions)) {
      questions.forEach(q => {
        // Sjekk at spørsmålet har både id og text
        if (q && q.id !== undefined && q.text) {
          addQuestionToList(q.id, q.text);
        } else {
          console.warn('Invalid question object:', q);
        }
      });
    } else {
      console.warn('Questions is not an array:', questions);
    }
  })
  .catch(err => {
    console.error('Error loading questions:', err);
    // Ikke vis alert ved oppstart, bare logg feilen
  });
}

// 🗑️ Slett alle spørsmål fra SPIFFS
function clearQuestions() {
  if (!confirm("Are you sure you want to delete all questions?")) {
    return;
  }

  fetch('/api/questions', {
    method: 'DELETE',
    headers: { 'Content-Type': 'application/json' }
  })
  .then(response => {
    if (!response.ok) throw new Error("Failed to clear questions");
    return response.json();
  })
  .then(() => {
    // Tøm visningen
    document.getElementById('questionList').innerHTML = '';
    alert("All questions deleted!");
  })
  .catch(err => {
    console.error('Error clearing questions:', err);
    alert("Error clearing questions from ESP32.");
  });
}

// 📤 Samler markerte spørsmål og sender som JSON til LCD
function sendToScreen() {
  const selectedQuestions = [];
  document.querySelectorAll('#questionList input[type="checkbox"]:checked')
    .forEach(cb => {
      const id = parseInt(cb.dataset.questionId);
      const text = cb.dataset.questionText;
      
      // Valider at vi har gyldige verdier
      if (!isNaN(id) && text) {
        selectedQuestions.push({
          id: id,
          text: text
        });
      } else {
        console.warn('Invalid question data:', { id: cb.dataset.questionId, text });
      }
    });

  if (selectedQuestions.length === 0) {
    alert("Please select at least one question.");
    return;
  }

  console.log('Sending to screen:', selectedQuestions);

  fetch('/api/screen', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ questions: selectedQuestions })
  })
  .then(response => {
    if (!response.ok) throw new Error("Failed to send data");
    return response.json();
  })
  .then(() => {
    alert("Sent to screen! " + selectedQuestions.length + " question(s) ready.");
  })
  .catch(err => {
    console.error(err);
    alert("Error sending data to ESP32.");
  });
}
