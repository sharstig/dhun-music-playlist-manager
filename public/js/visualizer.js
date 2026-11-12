/**
 * visualizer.js - Interactive Doubly Circular Linked List Visualizer
 * Real-time graphical representation of node memory, pointers, and circular links.
 */

class DhunVisualizer {
  constructor(chainContainerId, consoleLogId) {
    this.chainContainer = document.getElementById(chainContainerId);
    this.consoleLog = document.getElementById(consoleLogId);
    this.onNodeClickCallback = null;
  }

  log(message, type = 'info') {
    if (!this.consoleLog) return;
    const now = new Date();
    const timeStr = now.toTimeString().split(' ')[0];

    const line = document.createElement('div');
    line.className = `log-line ${type}`;
    line.textContent = `[${timeStr}] ${message}`;

    this.consoleLog.appendChild(line);
    this.consoleLog.scrollTop = this.consoleLog.scrollHeight;
  }

  clearLog() {
    if (this.consoleLog) {
      this.consoleLog.innerHTML = '<div class="log-line info">[LOG] Console cleared.</div>';
    }
  }

  render(dsaState) {
    if (!this.chainContainer) return;
    this.chainContainer.innerHTML = '';

    const nodes = dsaState.nodes || [];
    if (nodes.length === 0) {
      this.chainContainer.innerHTML = `
        <div class="empty-state" style="padding: 24px; min-width: 100%;">
          <p style="color: var(--text-dim); font-family: var(--font-mono);">
            [NULLPTR] DoublyCircularLinkedList is empty: head == tail == current == nullptr
          </p>
        </div>
      `;
      return;
    }

    // Render nodes sequentially
    nodes.forEach((node, index) => {
      const card = document.createElement('div');
      card.className = `visual-node-card ${node.isCurrent ? 'is-current' : ''} ${node.isHead ? 'is-head' : ''} ${node.isTail ? 'is-tail' : ''}`;
      card.title = `Click to set Current Song (ID: ${node.id})`;

      let badgeHtml = '';
      if (node.isHead) badgeHtml += '<span class="card-badge head">HEAD</span>';
      if (node.isTail) badgeHtml += '<span class="card-badge tail">TAIL</span>';
      if (node.isCurrent) badgeHtml += '<span class="card-badge current">PLAYING</span>';

      card.innerHTML = `
        <div class="node-card-header">
          <span class="node-id-tag">Node #${node.id}</span>
          <div class="node-badge-stack">${badgeHtml}</div>
        </div>
        <div class="node-card-body">
          <h4>${this.escapeHtml(node.title)}</h4>
          <p>${this.escapeHtml(node.artist)} • ${this.formatDuration(node.duration)}</p>
        </div>
        <div class="node-pointers-table">
          <div class="ptr-cell prev-ptr">
            <span class="ptr-label">&larr; prev ptr:</span>
            <span class="ptr-target">#${node.prevId}</span>
          </div>
          <div class="ptr-cell next-ptr">
            <span class="ptr-label">next ptr &rarr;:</span>
            <span class="ptr-target">#${node.nextId}</span>
          </div>
        </div>
      `;

      card.addEventListener('click', () => {
        if (this.onNodeClickCallback) {
          this.onNodeClickCallback(node.id);
        }
      });

      this.chainContainer.appendChild(card);

      // Render connector between adjacent nodes
      if (index < nodes.length - 1) {
        const connector = document.createElement('div');
        connector.className = 'node-connector';
        connector.innerHTML = `
          <div class="arrow-line forward" title="next pointer (${node.id} &rarr; ${nodes[index + 1].id})"></div>
          <div class="arrow-line backward" title="prev pointer (${nodes[index + 1].id} &larr; ${node.id})"></div>
        `;
        this.chainContainer.appendChild(connector);
      }
    });

    // Circular loopback connector indicator from tail back to head
    if (nodes.length > 1) {
      const loopConnector = document.createElement('div');
      loopConnector.className = 'node-connector circular-loop';
      loopConnector.style.minWidth = '80px';
      loopConnector.innerHTML = `
        <div style="font-size: 0.7rem; font-family: var(--font-mono); color: var(--accent-violet); text-align: center;">
          <span>&larr; tail&rarr;next = head &rarr;</span>
          <br>
          <span style="color: var(--accent-cyan);">&rarr; head&rarr;prev = tail &larr;</span>
        </div>
      `;
      this.chainContainer.appendChild(loopConnector);
    }
  }

  escapeHtml(str) {
    if (!str) return '';
    return str.replace(/[&<>"']/g, m => ({
      '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'
    }[m]));
  }

  formatDuration(seconds) {
    const mins = Math.floor(seconds / 60);
    const secs = seconds % 60;
    return `${mins}:${secs.toString().padStart(2, '0')}`;
  }
}

window.DhunVisualizer = DhunVisualizer;
