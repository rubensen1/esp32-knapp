
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