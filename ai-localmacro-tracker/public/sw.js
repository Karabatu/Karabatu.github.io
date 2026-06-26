const CACHE_NAME = 'nozio-cache-v1';

// Statische Core-Dateien für Offline-Betrieb vorab cachen
const STATIC_ASSETS = [
  './',
  './index.html',
  './manifest.json',
  'https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700;800&family=Outfit:wght@400;500;600;700;800;900&display=swap'
];

// Installation: Cache befüllen
self.addEventListener('install', (event) => {
  event.waitUntil(
    caches.open(CACHE_NAME).then((cache) => {
      console.log('[Service Worker] Installiert & Cache befüllt');
      return cache.addAll(STATIC_ASSETS);
    })
  );
  self.skipWaiting();
});

// Aktivierung: Alte Caches aufräumen
self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches.keys().then((keys) => {
      return Promise.all(
        keys.map((key) => {
          if (key !== CACHE_NAME) {
            console.log('[Service Worker] Lösche veralteten Cache:', key);
            return caches.delete(key);
          }
        })
      );
    })
  );
  self.clients.claim();
});

// Fetching: Cache-First-Strategie (außer für API-Aufrufe)
self.addEventListener('fetch', (event) => {
  const url = event.request.url;

  // API-Abfragen (Gemini, Open Food Facts) direkt überspringen - NICHT cachen!
  if (url.includes('googleapis.com') || url.includes('openfoodfacts.org')) {
    return; // Standard-Netzwerkanfrage zulassen
  }

  event.respondWith(
    caches.match(event.request).then((cachedResponse) => {
      if (cachedResponse) {
        return cachedResponse; // Aus dem Cache servieren
      }

      // Falls nicht im Cache, aus dem Netzwerk holen und dynamisch zwischenspeichern (außer für Drittanbieter)
      return fetch(event.request).then((networkResponse) => {
        if (!networkResponse || networkResponse.status !== 200 || networkResponse.type !== 'basic') {
          return networkResponse;
        }

        const responseToCache = networkResponse.clone();
        caches.open(CACHE_NAME).then((cache) => {
          cache.put(event.request, responseToCache);
        });

        return networkResponse;
      }).catch(() => {
        // Offline-Fallback für Navigationsanfragen
        if (event.request.mode === 'navigate') {
          return caches.match('./index.html');
        }
      });
    })
  );
});
