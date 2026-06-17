// UI Elements
const populationInput = document.getElementById('population');
const populationVal = document.getElementById('population-val');
const infectionProbInput = document.getElementById('infection-prob');
const infectionProbVal = document.getElementById('infection-prob-val');
const reproductionProbInput = document.getElementById('reproduction-prob');
const reproductionProbVal = document.getElementById('reproduction-prob-val');
const agentSpeedInput = document.getElementById('agent-speed');
const agentSpeedVal = document.getElementById('agent-speed-val');
const restartBtn = document.getElementById('restart-btn');
const livingEntitiesSpan = document.getElementById('living-entities');
const fpsSpan = document.getElementById('fps');

// Canvas Setup
const canvas = document.getElementById('sim-canvas');
const ctx = canvas.getContext('2d', { alpha: false }); // Optimize by disabling alpha
let width, height;

function resizeCanvas() {
    const container = document.getElementById('canvas-container');
    width = container.clientWidth;
    height = container.clientHeight;
    canvas.width = width;
    canvas.height = height;
}

window.addEventListener('resize', resizeCanvas);
resizeCanvas();

// Global Configuration
let config = {
    population: parseInt(populationInput.value),
    infectionProb: parseFloat(infectionProbInput.value) / 100,
    reproductionProb: parseFloat(reproductionProbInput.value) / 100,
    agentSpeed: parseFloat(agentSpeedInput.value)
};

// Update UI labels and config
function updateUI() {
    populationVal.textContent = populationInput.value;
    infectionProbVal.textContent = infectionProbInput.value + '%';
    reproductionProbVal.textContent = reproductionProbInput.value + '%';
    agentSpeedVal.textContent = agentSpeedInput.value;

    config.population = parseInt(populationInput.value);
    config.infectionProb = parseFloat(infectionProbInput.value) / 100;
    config.reproductionProb = parseFloat(reproductionProbInput.value) / 100;
    config.agentSpeed = parseFloat(agentSpeedInput.value);
}

populationInput.addEventListener('input', updateUI);
infectionProbInput.addEventListener('input', updateUI);
reproductionProbInput.addEventListener('input', updateUI);
agentSpeedInput.addEventListener('input', updateUI);

const AGENT_SIZE = 4;
const MAX_HEALTH = 300; // Frames an infected agent lives
const RECOVERY_CHANCE = 0.002; // Chance per frame to recover

class Agent {
    constructor(x, y, state = 'healthy') {
        this.x = x;
        this.y = y;
        this.state = state; // 'healthy', 'infected', 'immune'
        this.health = MAX_HEALTH;
        this.cooldown = 100; // Cooldown before reproduction is allowed

        // Random direction
        const angle = Math.random() * Math.PI * 2;
        this.vx = Math.cos(angle);
        this.vy = Math.sin(angle);
    }

    update(speed) {
        // Movement
        this.x += this.vx * speed;
        this.y += this.vy * speed;

        // Bouncing off edges
        if (this.x < 0) {
            this.x = 0;
            this.vx *= -1;
        } else if (this.x > width - AGENT_SIZE) {
            this.x = width - AGENT_SIZE;
            this.vx *= -1;
        }

        if (this.y < 0) {
            this.y = 0;
            this.vy *= -1;
        } else if (this.y > height - AGENT_SIZE) {
            this.y = height - AGENT_SIZE;
            this.vy *= -1;
        }

        // Reproduction cooldown
        if (this.cooldown > 0) {
            this.cooldown--;
        }

        // Infection logic
        if (this.state === 'infected') {
            this.health--;
            if (this.health > 0 && Math.random() < RECOVERY_CHANCE) {
                this.state = 'immune';
            }
        }
    }

    draw(ctx) {
        if (this.state === 'healthy') {
            ctx.fillStyle = '#00ff00';
        } else if (this.state === 'infected') {
            ctx.fillStyle = '#ff0000';
        } else if (this.state === 'immune') {
            ctx.fillStyle = '#0088ff';
        }
        ctx.fillRect(Math.floor(this.x), Math.floor(this.y), AGENT_SIZE, AGENT_SIZE);
    }
}

// Simulation State
let agents = [];
let animationId;
let lastTime = performance.now();
let frameCount = 0;

// Spatial Grid for optimized collision detection
const CELL_SIZE = 10; // Slightly larger than AGENT_SIZE to ensure collisions are caught
let grid = [];
let cols, rows;

function initGrid() {
    cols = Math.ceil(width / CELL_SIZE);
    rows = Math.ceil(height / CELL_SIZE);
    grid = new Array(cols * rows).fill(null).map(() => []);
}

function getGridIndex(x, y) {
    const col = Math.floor(x / CELL_SIZE);
    const row = Math.floor(y / CELL_SIZE);
    if (col < 0 || col >= cols || row < 0 || row >= rows) return -1;
    return row * cols + col;
}

function initSimulation() {
    if (animationId) cancelAnimationFrame(animationId);

    agents = [];
    initGrid();

    // Spawn initial population
    const initialInfectedCount = Math.max(1, Math.floor(config.population * 0.01)); // At least 1 infected, or 1%

    for (let i = 0; i < config.population; i++) {
        const x = Math.random() * (width - AGENT_SIZE);
        const y = Math.random() * (height - AGENT_SIZE);
        const state = i < initialInfectedCount ? 'infected' : 'healthy';
        agents.push(new Agent(x, y, state));
    }

    lastTime = performance.now();
    frameCount = 0;
    simulationLoop();
}

restartBtn.addEventListener('click', initSimulation);

function handleCollisions() {
    // Clear grid
    for (let i = 0; i < grid.length; i++) {
        grid[i].length = 0;
    }

    // Populate grid
    for (let i = 0; i < agents.length; i++) {
        const agent = agents[i];
        const idx = getGridIndex(agent.x, agent.y);
        if (idx !== -1) {
            grid[idx].push(agent);
        }
    }

    const newAgents = [];

    // Check collisions within cells and neighbor cells
    for (let row = 0; row < rows; row++) {
        for (let col = 0; col < cols; col++) {
            const idx = row * cols + col;
            const cellAgents = grid[idx];

            if (cellAgents.length === 0) continue;

            // Gather neighbors
            const neighbors = [];
            for (let dy = -1; dy <= 1; dy++) {
                for (let dx = -1; dx <= 1; dx++) {
                    const nRow = row + dy;
                    const nCol = col + dx;
                    if (nRow >= 0 && nRow < rows && nCol >= 0 && nCol < cols) {
                        const nIdx = nRow * cols + nCol;
                        for (let k = 0; k < grid[nIdx].length; k++) {
                            neighbors.push(grid[nIdx][k]);
                        }
                    }
                }
            }

            // Check collisions among gathered agents
            for (let i = 0; i < cellAgents.length; i++) {
                const a = cellAgents[i];
                for (let j = 0; j < neighbors.length; j++) {
                    const b = neighbors[j];
                    if (a === b) continue;

                    // Simple AABB Collision
                    if (a.x < b.x + AGENT_SIZE &&
                        a.x + AGENT_SIZE > b.x &&
                        a.y < b.y + AGENT_SIZE &&
                        a.y + AGENT_SIZE > b.y) {

                        // Infection
                        if ((a.state === 'healthy' && b.state === 'infected') ||
                            (a.state === 'infected' && b.state === 'healthy')) {
                            if (Math.random() < config.infectionProb) {
                                if (a.state === 'healthy') a.state = 'infected';
                                if (b.state === 'healthy') b.state = 'infected';
                            }
                        }

                        // Reproduction
                        if (a.state === 'healthy' && b.state === 'healthy' && a.cooldown === 0 && b.cooldown === 0) {
                            if (Math.random() < config.reproductionProb) {
                                a.cooldown = 200;
                                b.cooldown = 200;
                                // Spawn new agent near parents
                                const newX = Math.min(Math.max(0, a.x + (Math.random() * 10 - 5)), width - AGENT_SIZE);
                                const newY = Math.min(Math.max(0, a.y + (Math.random() * 10 - 5)), height - AGENT_SIZE);
                                newAgents.push(new Agent(newX, newY, 'healthy'));
                            }
                        }
                    }
                }
            }
        }
    }

    // Add newborn agents
    for (let i = 0; i < newAgents.length; i++) {
        agents.push(newAgents[i]);
    }
}

function simulationLoop() {
    ctx.fillStyle = '#000000'; // Clear background
    ctx.fillRect(0, 0, width, height);

    ctx.beginPath();

    // Process agents backwards to allow safe removal
    for (let i = agents.length - 1; i >= 0; i--) {
        const agent = agents[i];

        if (agent.state === 'infected' && agent.health <= 0) {
            agents.splice(i, 1);
            continue;
        }

        agent.update(config.agentSpeed);
        agent.draw(ctx);
    }

    handleCollisions();

    // Update Stats
    livingEntitiesSpan.textContent = agents.length;

    frameCount++;
    const currentTime = performance.now();
    if (currentTime - lastTime >= 1000) {
        fpsSpan.textContent = frameCount;
        frameCount = 0;
        lastTime = currentTime;
    }

    animationId = requestAnimationFrame(simulationLoop);
}

// Start initially
initSimulation();
