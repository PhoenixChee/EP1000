// Create function to select elements
const selectElement = (element) => document.querySelector(element);

// Open and close menubar on click, change nav elements
selectElement('.menu') .addEventListener('click', () => {
    selectElement('nav').classList.toggle('toggle');
});

// Open and close menubar on click, change header elements
selectElement('.menu') .addEventListener('click', () => {
    selectElement('header').classList.toggle('toggle');
});