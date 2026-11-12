/**
 * api.js - REST API Client for DHUN C++17 Backend
 * Connects frontend UI to native Doubly Circular Linked List endpoints.
 */

const API = {
  baseUrl: window.location.origin,

  async request(endpoint, options = {}) {
    const url = `${this.baseUrl}${endpoint}`;
    const defaultHeaders = {
      'Content-Type': 'application/json',
      'Accept': 'application/json'
    };

    const config = {
      ...options,
      headers: {
        ...defaultHeaders,
        ...options.headers
      }
    };

    try {
      const response = await fetch(url, config);
      const data = await response.json().catch(() => ({}));

      if (!response.ok) {
        throw new Error(data.error || `HTTP error ${response.status}`);
      }

      return data;
    } catch (err) {
      console.error(`[API ERROR] ${options.method || 'GET'} ${endpoint}:`, err);
      throw err;
    }
  },

  // 1. Get all songs in circular playlist order
  async getSongs() {
    return this.request('/api/songs');
  },

  // 2. Get current playing song node
  async getCurrentSong() {
    return this.request('/api/current');
  },

  // 3. Add new song to list (appends to tail)
  async addSong(songData) {
    return this.request('/api/songs', {
      method: 'POST',
      body: JSON.stringify(songData)
    });
  },

  // 4. Delete song by ID or Title
  async deleteSong({ id, title }) {
    let query = '';
    if (id) query = `?id=${encodeURIComponent(id)}`;
    else if (title) query = `?title=${encodeURIComponent(title)}`;
    return this.request(`/api/songs${query}`, {
      method: 'DELETE'
    });
  },

  // 5. Search playlist by query string
  async searchSongs(query) {
    return this.request(`/api/songs/search?q=${encodeURIComponent(query)}`);
  },

  // 6. Forward circular navigation
  async nextSong() {
    return this.request('/api/next', {
      method: 'POST'
    });
  },

  // 7. Backward circular navigation
  async prevSong() {
    return this.request('/api/prev', {
      method: 'POST'
    });
  },

  // 8. Jump to specific song node by ID
  async playSong(id) {
    return this.request('/api/play', {
      method: 'POST',
      body: JSON.stringify({ id })
    });
  },

  // 9. Reset playlist to default demo tracks
  async resetDemo() {
    return this.request('/api/reset', {
      method: 'POST'
    });
  },

  // 10. Real-time DSA pointer structure
  async getDsaState() {
    return this.request('/api/dsa-state');
  },

  // 11. Backend health check
  async getHealth() {
    return this.request('/health');
  }
};

window.API = API;
