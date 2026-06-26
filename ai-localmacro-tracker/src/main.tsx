import {StrictMode} from 'react';
import {createRoot} from 'react-dom/client';
import App from './App.tsx';
import './index.css';

// Service Worker für PWA Offline-Fähigkeit registrieren
if ('serviceWorker' in navigator && process.env.NODE_ENV === 'production') {
  window.addEventListener('load', () => {
    navigator.serviceWorker.register('./sw.js')
      .then((registration) => {
        console.log('Nozio Service Worker erfolgreich registriert:', registration.scope);
      })
      .catch((error) => {
        console.error('Service Worker Registrierung fehlgeschlagen:', error);
      });
  });
}

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <App />
  </StrictMode>,
);
