/**
 * app.js - Main Application Controller for DHUN
 * Manages state, UI synchronization, audio engine, visualizer, and user events.
 */

document.addEventListener('DOMContentLoaded', () => {
  // Application State
  const state = {
    songs: [],
    currentSong: null,
    songCount: 0,
    headId: -1,
    tailId: -1,
    searchQuery: '',
    isPlaying: false
  };

  // Instantiate Subsystems
  const audio = new DhunAudioEngine();
  const visualizer = new DhunVisualizer('nodes-chain', 'console-log');

  // DOM Elements - Sidebar & Status
  const sidebarCount = document.getElementById('sidebar-song-count');
  const monitorHead = document.getElementById('monitor-head');
  const monitorTail = document.getElementById('monitor-tail');
  const monitorCurrent = document.getElementById('monitor-current');
  const monitorLoop = document.getElementById('monitor-loop');
  const statusDot = document.getElementById('status-dot');
  const statusText = document.getElementById('status-text');

  // DOM Elements - Hero
  const heroBanner = document.getElementById('hero-banner');
  const heroCoverArt = document.getElementById('hero-cover-art');
  const heroCoverWrapper = document.querySelector('.hero-cover-wrapper');
  const heroGradient = document.getElementById('hero-gradient');
  const heroNodeId = document.getElementById('hero-node-id');
  const heroNodePos = document.getElementById('hero-node-position');
  const heroTitle = document.getElementById('hero-title');
  const heroArtist = document.getElementById('hero-artist');
  const heroPrevNode = document.getElementById('hero-prev-node');
  const heroNextNode = document.getElementById('hero-next-node');
  const equalizerBars = document.getElementById('equalizer-bars');

  // DOM Elements - Table
  const playlistTbody = document.getElementById('playlist-tbody');
  const emptyState = document.getElementById('empty-state');
  const statSongCount = document.getElementById('stat-song-count');
  const statTotalTime = document.getElementById('stat-total-time');

  // DOM Elements - Player Bar
  const playerCoverArt = document.getElementById('player-cover-art');
  const playerTitle = document.getElementById('player-title');
  const playerArtist = document.getElementById('player-artist');
  const ctrlPrev = document.getElementById('ctrl-prev');
  const ctrlPlay = document.getElementById('ctrl-play');
  const ctrlNext = document.getElementById('ctrl-next');
  const iconPlay = document.getElementById('icon-play');
  const iconPause = document.getElementById('icon-pause');
  const timeCurrent = document.getElementById('time-current');
  const timeTotal = document.getElementById('time-total');
  const progressTrack = document.getElementById('progress-track');
  const progressFill = document.getElementById('progress-fill');
  const volumeSlider = document.getElementById('volume-slider');
  const btnVolumeIcon = document.getElementById('btn-volume-icon');
  const iconVolHigh = document.getElementById('icon-vol-high');
  const iconVolMute = document.getElementById('icon-vol-mute');

  // DOM Elements - Search & Modals
  const searchInput = document.getElementById('search-input');
  const searchClear = document.getElementById('search-clear');
  const modalAdd = document.getElementById('modal-add-song');
  const modalDelete = document.getElementById('modal-delete-song');
  const formAdd = document.getElementById('form-add-song');
  const formDelete = document.getElementById('form-delete-song');

  // Helper: Format Seconds -> "M:SS"
  function formatTime(seconds) {
    if (isNaN(seconds) || seconds < 0) return '0:00';
    const m = Math.floor(seconds / 60);
    const s = Math.floor(seconds % 60);
    return `${m}:${s.toString().padStart(2, '0')}`;
  }

  // Helper: Show Toast Notification
  function showToast(message, type = 'info') {
    const container = document.getElementById('toast-container');
    if (!container) return;

    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.innerHTML = `<span>${message}</span>`;

    container.appendChild(toast);
    setTimeout(() => {
      toast.style.opacity = '0';
      toast.style.transform = 'translateX(30px)';
      toast.style.transition = 'all 0.3s ease';
      setTimeout(() => toast.remove(), 300);
    }, 3500);
  }

  // Theme Icon / Color Presets
  const themeIcons = {
    sunset: '🌅',
    neon: '⚡',
    monsoon: '🌧️',
    aurora: '🌌',
    coffee: '☕',
    galaxy: '🪐'
  };

  // --- CORE REFRESH & STATE SYNCHRONIZATION ---
  async function refreshPlaylist(keepPlaying = true) {
    try {
      const data = await API.getSongs();
      state.songs = data.songs || [];
      state.songCount = data.count || 0;
      state.headId = data.headId;
      state.tailId = data.tailId;

      // Update sidebar & stats badges
      if (sidebarCount) sidebarCount.textContent = state.songCount;
      if (statSongCount) statSongCount.textContent = state.songCount;

      const totalSecs = state.songs.reduce((acc, s) => acc + (s.duration || 0), 0);
      const totalMins = Math.floor(totalSecs / 60);
      const remSecs = totalSecs % 60;
      if (statTotalTime) statTotalTime.textContent = `${totalMins}m ${remSecs}s`;

      // Update Invariant Monitor
      if (monitorHead) monitorHead.textContent = state.headId > 0 ? `#${state.headId}` : 'nullptr';
      if (monitorTail) monitorTail.textContent = state.tailId > 0 ? `#${state.tailId}` : 'nullptr';

      // Check current song
      const currentSongData = state.songs.find(s => s.id === data.currentId) || null;
      state.currentSong = currentSongData;

      if (monitorCurrent) {
        monitorCurrent.textContent = currentSongData ? `#${currentSongData.id}` : 'nullptr';
      }

      if (monitorLoop) {
        if (state.songCount === 0) {
          monitorLoop.textContent = 'Empty List (nullptr)';
          monitorLoop.className = 'value code';
        } else if (state.songCount === 1) {
          monitorLoop.textContent = 'Single Node (Self-Loop)';
          monitorLoop.className = 'value tag-ok';
        } else {
          monitorLoop.textContent = `tail(#${state.tailId}) ⇄ head(#${state.headId}) OK`;
          monitorLoop.className = 'value tag-ok';
        }
      }

      // Render Table & Visualizer
      renderTable(state.songs);
      updateHeroAndPlayer(state.currentSong, keepPlaying);

      // Render Visualizer Graph
      const dsaState = await API.getDsaState();
      visualizer.render(dsaState);

      // Update backend status indicator
      if (statusDot) statusDot.className = 'status-dot online';
      if (statusText) statusText.textContent = 'C++17 Backend Online';
    } catch (err) {
      console.error('Failed to sync playlist:', err);
      if (statusDot) statusDot.className = 'status-dot';
      if (statusText) statusText.textContent = 'Backend Disconnected';
      showToast('Cannot connect to C++ backend. Is the server running?', 'error');
    }
  }

  // --- RENDER PLAYLIST TABLE ---
  function renderTable(songs) {
    if (!playlistTbody) return;
    playlistTbody.innerHTML = '';

    if (!songs || songs.length === 0) {
      if (emptyState) emptyState.classList.remove('hidden');
      return;
    }

    if (emptyState) emptyState.classList.add('hidden');

    songs.forEach((song, idx) => {
      const tr = document.createElement('tr');
      tr.className = `song-row ${song.isCurrent ? 'active' : ''}`;
      tr.dataset.id = song.id;

      const theme = song.coverUrl || 'sunset';
      const icon = themeIcons[theme] || '🎵';

      let headTailChip = '';
      if (song.isHead) headTailChip += '<span class="node-chip head">HEAD</span> ';
      if (song.isTail) headTailChip += '<span class="node-chip tail">TAIL</span>';

      tr.innerHTML = `
        <td class="track-num">${idx + 1}</td>
        <td>
          <span class="node-chip">#${song.id}</span>
          ${headTailChip}
        </td>
        <td>
          <div class="song-cell-title">
            <div class="song-mini-art ${theme}">${icon}</div>
            <div class="song-name-group">
              <span class="title-text">
                ${visualizer.escapeHtml(song.title)}
                ${song.isCurrent ? '<span class="now-playing-pill">PLAYING</span>' : ''}
              </span>
              <span class="artist-text">${visualizer.escapeHtml(song.artist)}</span>
            </div>
          </div>
        </td>
        <td><span class="code" style="color: var(--accent-cyan); font-family: var(--font-mono);">&larr; #${song.prevId}</span></td>
        <td><span class="code" style="color: var(--accent-indigo); font-family: var(--font-mono);">#${song.nextId} &rarr;</span></td>
        <td><span style="font-family: var(--font-mono);">${formatTime(song.duration)}</span></td>
        <td>
          <div class="row-actions">
            <button class="action-btn btn-play" title="Set Current and Play Node #${song.id}">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor"><polygon points="5 3 19 12 5 21 5 3"></polygon></svg>
            </button>
            <button class="action-btn btn-del" title="Delete Node #${song.id}">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"></polyline><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"></path></svg>
            </button>
          </div>
        </td>
      `;

      // Row Play Button
      tr.querySelector('.btn-play').addEventListener('click', (e) => {
        e.stopPropagation();
        handlePlaySong(song.id);
      });

      // Row Delete Button
      tr.querySelector('.btn-del').addEventListener('click', async (e) => {
        e.stopPropagation();
        if (confirm(`Delete song "${song.title}" (Node ID #${song.id}) from playlist?`)) {
          try {
            await API.deleteSong({ id: song.id });
            visualizer.log(`Deleted node #${song.id} (${song.title}). Pointers re-linked.`, 'warning');
            showToast(`Deleted "${song.title}" successfully`, 'success');
            await refreshPlaylist();
          } catch (err) {
            showToast(err.message, 'error');
          }
        }
      });

      // Clicking row directly selects and plays the song
      tr.addEventListener('click', () => {
        handlePlaySong(song.id);
      });

      playlistTbody.appendChild(tr);
    });
  }

  // --- UPDATE HERO & AUDIO PLAYER DISPLAY ---
  function updateHeroAndPlayer(song, keepPlayingState = false) {
    if (!song) {
      if (heroTitle) heroTitle.textContent = 'Playlist is Empty';
      if (heroArtist) heroArtist.textContent = 'Add songs to begin playback';
      if (heroNodeId) heroNodeId.textContent = 'ID: None';
      if (heroNodePos) heroNodePos.textContent = 'No Head/Tail';
      if (heroPrevNode) heroPrevNode.textContent = '--';
      if (heroNextNode) heroNextNode.textContent = '--';
      if (playerTitle) playerTitle.textContent = 'No Song Selected';
      if (playerArtist) playerArtist.textContent = 'Playlist is empty';
      if (playerCoverArt) playerCoverArt.textContent = '🎵';
      if (timeTotal) timeTotal.textContent = '0:00';
      if (timeCurrent) timeCurrent.textContent = '0:00';
      if (progressFill) progressFill.style.width = '0%';
      audio.loadSong({ duration: 0 });
      return;
    }

    const theme = song.coverUrl || 'sunset';
    const icon = themeIcons[theme] || '🎵';

    // Hero Updates
    if (heroTitle) heroTitle.textContent = song.title;
    if (heroArtist) heroArtist.textContent = song.artist;
    if (heroNodeId) heroNodeId.textContent = `Node ID: #${song.id}`;
    if (heroNodePos) {
      if (song.isHead && song.isTail) heroNodePos.textContent = 'Sole Node (Head & Tail)';
      else if (song.isHead) heroNodePos.textContent = 'Head Node (First in List)';
      else if (song.isTail) heroNodePos.textContent = 'Tail Node (Last in List)';
      else heroNodePos.textContent = 'Internal Node';
    }
    if (heroPrevNode) heroPrevNode.textContent = `#${song.prevId} (${song.isHead ? 'Circular from Tail' : 'Previous'})`;
    if (heroNextNode) heroNextNode.textContent = `#${song.nextId} (${song.isTail ? 'Circular to Head' : 'Next'})`;

    if (heroGradient) {
      heroGradient.className = `cover-gradient ${theme}`;
      heroGradient.textContent = icon;
    }

    // Player Bar Updates
    if (playerTitle) playerTitle.textContent = song.title;
    if (playerArtist) playerArtist.textContent = song.artist;
    if (playerCoverArt) {
      playerCoverArt.className = `player-cover ${theme}`;
      playerCoverArt.textContent = icon;
    }
    if (timeTotal) timeTotal.textContent = formatTime(song.duration);

    // Audio Engine Sync
    audio.loadSong(song);
    if (keepPlayingState && state.isPlaying) {
      audio.play();
    }
  }

  // --- AUDIO ENGINE EVENT BINDINGS ---
  audio.onProgressCallback = (current, total) => {
    if (timeCurrent) timeCurrent.textContent = formatTime(current);
    if (timeTotal) timeTotal.textContent = formatTime(total);
    if (progressFill && total > 0) {
      const pct = (current / total) * 100;
      progressFill.style.width = `${pct}%`;
    }
  };

  audio.onStateChangeCallback = (playing) => {
    state.isPlaying = playing;
    if (playing) {
      if (iconPlay) iconPlay.classList.add('hidden');
      if (iconPause) iconPause.classList.remove('hidden');
      if (equalizerBars) equalizerBars.classList.add('animating');
      if (heroCoverWrapper) heroCoverWrapper.classList.add('playing');
    } else {
      if (iconPlay) iconPlay.classList.remove('hidden');
      if (iconPause) iconPause.classList.add('hidden');
      if (equalizerBars) equalizerBars.classList.remove('animating');
      if (heroCoverWrapper) heroCoverWrapper.classList.remove('playing');
    }
  };

  // When song naturally completes duration, circularly loop to next song!
  audio.onEndCallback = async () => {
    visualizer.log(`Track finished. Circular auto-advancing to next song...`, 'info');
    await handleNextSong();
  };

  // --- USER INTERACTION HANDLERS ---
  async function handlePlaySong(id) {
    try {
      const song = await API.playSong(id);
      visualizer.log(`Set current song to #${song.id} ("${song.title}").`, 'info');
      state.currentSong = song;
      await refreshPlaylist(false);
      audio.play();
      showToast(`Now playing: ${song.title}`, 'info');
    } catch (err) {
      showToast(err.message, 'error');
    }
  }

  async function handleNextSong() {
    try {
      const song = await API.nextSong();
      visualizer.log(`Advanced to next node #${song.id} ("${song.title}"). Circular traversal: tail -> head preserved.`, 'info');
      state.currentSong = song;
      await refreshPlaylist(false);
      audio.play();
      showToast(`Next: ${song.title}`, 'info');
    } catch (err) {
      showToast(err.message, 'error');
    }
  }

  async function handlePrevSong() {
    try {
      const song = await API.prevSong();
      visualizer.log(`Navigated to prev node #${song.id} ("${song.title}"). Circular traversal: head -> tail preserved.`, 'info');
      state.currentSong = song;
      await refreshPlaylist(false);
      audio.play();
      showToast(`Previous: ${song.title}`, 'info');
    } catch (err) {
      showToast(err.message, 'error');
    }
  }

  // Play / Pause Toggle
  if (ctrlPlay) {
    ctrlPlay.addEventListener('click', () => {
      if (!state.currentSong && state.songs.length > 0) {
        handlePlaySong(state.songs[0].id);
      } else {
        audio.togglePlay();
      }
    });
  }

  if (ctrlNext) ctrlNext.addEventListener('click', handleNextSong);
  if (ctrlPrev) ctrlPrev.addEventListener('click', handlePrevSong);

  // Progress Bar Seek
  if (progressTrack) {
    progressTrack.addEventListener('click', (e) => {
      const rect = progressTrack.getBoundingClientRect();
      const clickX = e.clientX - rect.left;
      const width = rect.width;
      const pct = Math.max(0, Math.min(1, clickX / width));
      if (audio.duration > 0) {
        audio.seek(pct * audio.duration);
      }
    });
  }

  // Volume Slider
  if (volumeSlider) {
    volumeSlider.addEventListener('input', (e) => {
      const val = parseFloat(e.target.value);
      audio.setVolume(val);
      if (val === 0) {
        if (iconVolHigh) iconVolHigh.classList.add('hidden');
        if (iconVolMute) iconVolMute.classList.remove('hidden');
      } else {
        if (iconVolHigh) iconVolHigh.classList.remove('hidden');
        if (iconVolMute) iconVolMute.classList.add('hidden');
      }
    });
  }

  // Mute Button
  if (btnVolumeIcon) {
    btnVolumeIcon.addEventListener('click', () => {
      const muted = audio.toggleMute();
      if (muted) {
        if (iconVolHigh) iconVolHigh.classList.add('hidden');
        if (iconVolMute) iconVolMute.classList.remove('hidden');
      } else {
        if (iconVolHigh) iconVolHigh.classList.remove('hidden');
        if (iconVolMute) iconVolMute.classList.add('hidden');
      }
    });
  }

  // --- SEARCH BAR (WITH DEBOUNCE) ---
  let searchTimeout = null;
  if (searchInput) {
    searchInput.addEventListener('input', (e) => {
      const q = e.target.value.trim();
      state.searchQuery = q;

      if (q.length > 0) {
        if (searchClear) searchClear.classList.remove('hidden');
      } else {
        if (searchClear) searchClear.classList.add('hidden');
      }

      clearTimeout(searchTimeout);
      searchTimeout = setTimeout(async () => {
        if (q === '') {
          renderTable(state.songs);
        } else {
          try {
            const data = await API.searchSongs(q);
            renderTable(data.songs || []);
            visualizer.log(`Searched for "${q}": found ${data.songs ? data.songs.length : 0} match(es).`, 'info');
          } catch (err) {
            renderTable([]);
          }
        }
      }, 250);
    });
  }

  if (searchClear) {
    searchClear.addEventListener('click', () => {
      if (searchInput) searchInput.value = '';
      state.searchQuery = '';
      searchClear.classList.add('hidden');
      renderTable(state.songs);
    });
  }

  // --- MODALS (ADD & DELETE) ---
  const btnOpenAdd = document.getElementById('btn-open-add-modal');
  const btnHeaderAdd = document.getElementById('btn-header-add');
  const btnEmptyAdd = document.getElementById('btn-empty-add');
  const modalAddClose = document.getElementById('modal-add-close');
  const btnAddCancel = document.getElementById('btn-add-cancel');

  const openAddModal = () => { if (modalAdd) modalAdd.classList.remove('hidden'); };
  const closeAddModal = () => { if (modalAdd) modalAdd.classList.add('hidden'); };

  if (btnOpenAdd) btnOpenAdd.addEventListener('click', openAddModal);
  if (btnHeaderAdd) btnHeaderAdd.addEventListener('click', openAddModal);
  if (btnEmptyAdd) btnEmptyAdd.addEventListener('click', openAddModal);
  if (modalAddClose) modalAddClose.addEventListener('click', closeAddModal);
  if (btnAddCancel) btnAddCancel.addEventListener('click', closeAddModal);

  // Submit Add Song Form
  if (formAdd) {
    formAdd.addEventListener('submit', async (e) => {
      e.preventDefault();
      const title = document.getElementById('input-add-title').value.trim();
      const artist = document.getElementById('input-add-artist').value.trim();
      const duration = parseInt(document.getElementById('input-add-duration').value, 10);
      const customIdVal = document.getElementById('input-add-id').value.trim();
      const customId = customIdVal ? parseInt(customIdVal, 10) : 0;
      const theme = document.getElementById('select-add-theme').value;

      try {
        const payload = { title, artist, duration, coverUrl: theme };
        if (customId > 0) payload.id = customId;

        const newSong = await API.addSong(payload);
        visualizer.log(`Added song node #${newSong.id} ("${newSong.title}") to tail of linked list.`, 'success');
        showToast(`Added "${title}" (Node #${newSong.id})`, 'success');

        closeAddModal();
        formAdd.reset();
        await refreshPlaylist();
      } catch (err) {
        showToast(err.message, 'error');
      }
    });
  }

  // Delete Modal
  const btnOpenDelete = document.getElementById('btn-open-delete-modal');
  const modalDelClose = document.getElementById('modal-del-close');
  const btnDelCancel = document.getElementById('btn-del-cancel');

  const openDelModal = () => { if (modalDelete) modalDelete.classList.remove('hidden'); };
  const closeDelModal = () => { if (modalDelete) modalDelete.classList.add('hidden'); };

  if (btnOpenDelete) btnOpenDelete.addEventListener('click', openDelModal);
  if (modalDelClose) modalDelClose.addEventListener('click', closeDelModal);
  if (btnDelCancel) btnDelCancel.addEventListener('click', closeDelModal);

  // Submit Delete Form
  if (formDelete) {
    formDelete.addEventListener('submit', async (e) => {
      e.preventDefault();
      const idVal = document.getElementById('input-del-id').value.trim();
      const titleVal = document.getElementById('input-del-title').value.trim();

      if (!idVal && !titleVal) {
        showToast('Please provide either Song ID or Title to delete.', 'error');
        return;
      }

      try {
        await API.deleteSong({ id: idVal, title: titleVal });
        visualizer.log(`Deleted song (${idVal ? 'ID #' + idVal : '"' + titleVal + '"'}). Reconnected neighbor pointers.`, 'warning');
        showToast('Song deleted successfully', 'success');

        closeDelModal();
        formDelete.reset();
        await refreshPlaylist();
      } catch (err) {
        showToast(err.message, 'error');
      }
    });
  }

  // Reset Demo Tracks Button
  const btnResetDemo = document.getElementById('btn-reset-demo');
  if (btnResetDemo) {
    btnResetDemo.addEventListener('click', async () => {
      if (confirm('Reset playlist back to default demo tracks?')) {
        try {
          await API.resetDemo();
          visualizer.log('Playlist reset to initial 6 demo tracks.', 'info');
          showToast('Playlist reset to default demo tracks', 'success');
          await refreshPlaylist();
        } catch (err) {
          showToast(err.message, 'error');
        }
      }
    });
  }

  // Clear Console Log Button
  const btnClearConsole = document.getElementById('console-clear');
  if (btnClearConsole) {
    btnClearConsole.addEventListener('click', () => visualizer.clearLog());
  }

  // Visualizer Node Click Callback
  visualizer.onNodeClickCallback = (nodeId) => {
    handlePlaySong(nodeId);
  };

  // --- TAB NAVIGATION ---
  const navItems = document.querySelectorAll('.nav-item');
  const tabViews = document.querySelectorAll('.tab-view');

  navItems.forEach(item => {
    item.addEventListener('click', () => {
      const targetId = item.dataset.tab;
      navItems.forEach(n => n.classList.remove('active'));
      tabViews.forEach(v => v.classList.remove('active'));

      item.classList.add('active');
      const targetView = document.getElementById(targetId);
      if (targetView) targetView.classList.add('active');

      if (targetId === 'visualizer-view') {
        API.getDsaState().then(dsaState => visualizer.render(dsaState));
      }

      // Close mobile sidebar drawer when a tab is selected
      if (sidebar && sidebar.classList.contains('open')) {
        sidebar.classList.remove('open');
        if (sidebarBackdrop) sidebarBackdrop.classList.remove('open');
      }
    });
  });

  // Mobile Menu Drawer Toggle
  const sidebar = document.getElementById('sidebar');
  const sidebarBackdrop = document.getElementById('sidebar-backdrop');
  const mobileMenuBtn = document.getElementById('mobile-menu-btn');
  const sidebarCloseBtn = document.getElementById('sidebar-close-btn');

  if (mobileMenuBtn && sidebar) {
    mobileMenuBtn.addEventListener('click', () => {
      sidebar.classList.add('open');
      if (sidebarBackdrop) sidebarBackdrop.classList.add('open');
    });
  }

  const closeMobileSidebar = () => {
    if (sidebar) sidebar.classList.remove('open');
    if (sidebarBackdrop) sidebarBackdrop.classList.remove('open');
  };

  if (sidebarCloseBtn) sidebarCloseBtn.addEventListener('click', closeMobileSidebar);
  if (sidebarBackdrop) sidebarBackdrop.addEventListener('click', closeMobileSidebar);

  // Keyboard Shortcuts: Space (Play/Pause), Left Arrow (Prev), Right Arrow (Next)
  document.addEventListener('keydown', (e) => {
    if (['INPUT', 'SELECT', 'TEXTAREA'].includes(document.activeElement.tagName)) {
      return;
    }
    if (e.code === 'Space') {
      e.preventDefault();
      audio.togglePlay();
    } else if (e.code === 'ArrowRight') {
      e.preventDefault();
      handleNextSong();
    } else if (e.code === 'ArrowLeft') {
      e.preventDefault();
      handlePrevSong();
    }
  });

  // Initial Boot
  refreshPlaylist();
});
