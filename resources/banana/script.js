document.addEventListener('DOMContentLoaded', function () {
    let bananaCount = 0;
    const bananaCountElement = document.getElementById('bananaCount');
    const increaseButton = document.getElementById('increaseButton');

    increaseButton.addEventListener('click', function () {
        // Increase the banana count
        bananaCount++;
        bananaCountElement.textContent = bananaCount;

        // Create a falling banana
        createFallingBanana();
    });

    function createFallingBanana() {
        const banana = document.createElement('img');
        banana.src = 'fallingb.png';
        banana.className = 'falling-banana';
        banana.style.left = Math.random() * window.innerWidth + 'px'; // Random horizontal position

        document.body.appendChild(banana);

        // Remove the banana after it falls
        banana.addEventListener('animationend', function () {
            banana.remove();
        });
    }
});
