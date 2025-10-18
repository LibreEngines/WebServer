/**
 * Advanced C Web Server - Client-side JavaScript
 * Provides interactive functionality and API communication
 */

// Application state
const AppState = {
    serverStatus: 'unknown',
    lastUpdate: null,
    autoRefresh: false
};

// DOM Content Loaded Event
document.addEventListener('DOMContentLoaded', function() {
    initializeApp();
    setupEventListeners();
    checkServerStatus();
    
    // Add fade-in animation to main content
    const mainContent = document.querySelector('main');
    if (mainContent) {
        mainContent.classList.add('fade-in');
    }
});

/**
 * Initialize the application
 */
function initializeApp() {
    console.log('Advanced C Web Server - Client Application Initialized');
    
    // Add current time to footer
    updateFooterTime();
    
    // Set up periodic updates
    setInterval(updateFooterTime, 1000);
}

/**
 * Setup event listeners for interactive elements
 */
function setupEventListeners() {
    // Form submission enhancement
    const forms = document.querySelectorAll('form');
    forms.forEach(form => {
        form.addEventListener('submit', handleFormSubmission);
    });
    
    // Button click animations
    const buttons = document.querySelectorAll('.btn');
    buttons.forEach(button => {
        button.addEventListener('click', function(e) {
            addButtonClickEffect(e.target);
        });
    });
    
    // Add auto-refresh toggle
    // addAutoRefreshControl();
    
    // Add keyboard shortcuts
    setupKeyboardShortcuts();
}

/**
 * Handle form submissions with enhanced UX
 */
function handleFormSubmission(event) {
    const form = event.target;
    const submitBtn = form.querySelector('button[type="submit"]');
    
    if (submitBtn) {
        // Add loading state
        const originalText = submitBtn.innerHTML;
        submitBtn.innerHTML = '<i class="fas fa-spinner fa-spin me-2"></i>Submitting...';
        submitBtn.disabled = true;
        
        // Re-enable button after a short delay (for demonstration)
        setTimeout(() => {
            submitBtn.innerHTML = originalText;
            submitBtn.disabled = false;
        }, 1000);
    }
}

/**
 * Add visual click effect to buttons
 */
function addButtonClickEffect(button) {
    button.style.transform = 'scale(0.95)';
    setTimeout(() => {
        button.style.transform = '';
    }, 150);
}

/**
 * Check server status via API
 */
async function checkServerStatus() {
    try {
        const response = await fetch('/api/status');
        const data = await response.json();
        
        AppState.serverStatus = data.status || 'unknown';
        AppState.lastUpdate = new Date();
        
        updateStatusIndicators(data);
        
        console.log('Server Status:', data);
    } catch (error) {
        console.error('Failed to check server status:', error);
        AppState.serverStatus = 'error';
        updateStatusIndicators({ status: 'error', error: error.message });
    }
}

/**
 * Update status indicators in the UI
 */
function updateStatusIndicators(statusData) {
    // Create or update status indicator
    let statusIndicator = document.getElementById('status-indicator');
    
    if (!statusIndicator) {
        statusIndicator = document.createElement('div');
        statusIndicator.id = 'status-indicator';
        statusIndicator.className = 'position-fixed top-0 end-0 m-3';
        statusIndicator.style.zIndex = '1050';
        document.body.appendChild(statusIndicator);
    }
    
    const statusClass = getStatusClass(statusData.status);
    const statusIcon = getStatusIcon(statusData.status);
    
    statusIndicator.innerHTML = `
        <div class="alert ${statusClass} alert-dismissible fade show" role="alert">
            <i class="${statusIcon} me-2"></i>
            <strong>Server Status:</strong> ${statusData.status}
            ${statusData.version ? `<br><small>Version: ${statusData.version}</small>` : ''}
            <button type="button" class="btn-close" data-bs-dismiss="alert"></button>
        </div>
    `;
    
    // Auto-hide after 5 seconds
    setTimeout(() => {
        const alert = statusIndicator.querySelector('.alert');
        if (alert) {
            const bsAlert = new bootstrap.Alert(alert);
            bsAlert.close();
        }
    }, 5000);
}

/**
 * Get appropriate CSS class for status
 */
function getStatusClass(status) {
    switch (status) {
        case 'running':
            return 'alert-success';
        case 'error':
            return 'alert-danger';
        case 'warning':
            return 'alert-warning';
        default:
            return 'alert-info';
    }
}

/**
 * Get appropriate icon for status
 */
function getStatusIcon(status) {
    switch (status) {
        case 'running':
            return 'fas fa-check-circle status-online';
        case 'error':
            return 'fas fa-exclamation-circle status-offline';
        case 'warning':
            return 'fas fa-exclamation-triangle status-warning';
        default:
            return 'fas fa-info-circle';
    }
}

/**
 * Add auto-refresh control
 */
function addAutoRefreshControl() {
    const navbar = document.querySelector('.navbar');
    if (navbar) {
        const autoRefreshControl = document.createElement('div');
        autoRefreshControl.className = 'navbar-text me-3';
        autoRefreshControl.innerHTML = `
            <div class="form-check form-switch">
                <input class="form-check-input" type="checkbox" id="autoRefreshToggle">
                <label class="form-check-label text-light" for="autoRefreshToggle">
                    Auto-refresh
                </label>
            </div>
        `;
        
        const navbarNav = navbar.querySelector('.navbar-nav');
        if (navbarNav) {
            navbar.querySelector('.container').insertBefore(autoRefreshControl, navbarNav);
        }
        
        // Setup auto-refresh functionality
        const autoRefreshToggle = document.getElementById('autoRefreshToggle');
        autoRefreshToggle.addEventListener('change', function() {
            AppState.autoRefresh = this.checked;
            
            if (AppState.autoRefresh) {
                AppState.refreshInterval = setInterval(checkServerStatus, 30000); // 30 seconds
                showNotification('Auto-refresh enabled', 'info');
            } else {
                if (AppState.refreshInterval) {
                    clearInterval(AppState.refreshInterval);
                }
                showNotification('Auto-refresh disabled', 'info');
            }
        });
    }
}

/**
 * Setup keyboard shortcuts
 */
function setupKeyboardShortcuts() {
    document.addEventListener('keydown', function(event) {
        // Ctrl/Cmd + R: Refresh status
        if ((event.ctrlKey || event.metaKey) && event.key === 'r') {
            event.preventDefault();
            checkServerStatus();
            showNotification('Status refreshed', 'success');
        }
        
        // Ctrl/Cmd + H: Go to home
        if ((event.ctrlKey || event.metaKey) && event.key === 'h') {
            event.preventDefault();
            window.location.href = '/';
        }
    });
}

/**
 * Show notification to user
 */
function showNotification(message, type = 'info') {
    const notification = document.createElement('div');
    notification.className = `alert alert-${type} position-fixed bottom-0 end-0 m-3 fade show`;
    notification.style.zIndex = '1060';
    notification.innerHTML = `
        <i class="fas fa-info-circle me-2"></i>
        ${message}
        <button type="button" class="btn-close" data-bs-dismiss="alert"></button>
    `;
    
    document.body.appendChild(notification);
    
    // Auto-remove after 3 seconds
    setTimeout(() => {
        if (notification.parentNode) {
            const bsAlert = new bootstrap.Alert(notification);
            bsAlert.close();
        }
    }, 3000);
}

/**
 * Update footer with current time
 */
function updateFooterTime() {
    const footer = document.querySelector('footer p');
    if (footer && AppState.lastUpdate) {
        const timeStr = AppState.lastUpdate.toLocaleTimeString();
        const existingText = footer.textContent.split(' - Last updated:')[0];
        footer.innerHTML = `
            ${existingText} - Last updated: ${timeStr}
            ${AppState.serverStatus !== 'unknown' ? 
                `<span class="ms-2 badge bg-${AppState.serverStatus === 'running' ? 'success' : 'danger'}">${AppState.serverStatus}</span>` : ''
            }
        `;
    }
}

/**
 * API Helper functions
 */
const API = {
    /**
     * Make a GET request to the server
     */
    async get(endpoint) {
        try {
            const response = await fetch(endpoint);
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            return await response.json();
        } catch (error) {
            console.error(`GET ${endpoint} failed:`, error);
            throw error;
        }
    },
    
    /**
     * Make a POST request to the server
     */
    async post(endpoint, data) {
        try {
            const response = await fetch(endpoint, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(data)
            });
            
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            
            return await response.json();
        } catch (error) {
            console.error(`POST ${endpoint} failed:`, error);
            throw error;
        }
    }
};

/**
 * Utility functions
 */
const Utils = {
    /**
     * Format bytes to human readable format
     */
    formatBytes(bytes, decimals = 2) {
        if (bytes === 0) return '0 Bytes';
        
        const k = 1024;
        const dm = decimals < 0 ? 0 : decimals;
        const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
        
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        
        return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
    },
    
    /**
     * Format timestamp to relative time
     */
    timeAgo(date) {
        const now = new Date();
        const diffInSeconds = Math.floor((now - date) / 1000);
        
        if (diffInSeconds < 60) return 'just now';
        if (diffInSeconds < 3600) return `${Math.floor(diffInSeconds / 60)} minutes ago`;
        if (diffInSeconds < 86400) return `${Math.floor(diffInSeconds / 3600)} hours ago`;
        
        return date.toLocaleDateString();
    }
};

// Export for use in other scripts
window.AppState = AppState;
window.API = API;
window.Utils = Utils;

