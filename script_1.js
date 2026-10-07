
        // In-memory application state
        let state = {
            temp: 24.5,
            humidity: 55,
            light: 7,
            smoke_detected: 0,
            valve_open: true,
            settings: {
                temp_warning: 28.0,
                temp_danger: 35.0,
                hum_warning: 70,
                hum_danger: 85
            },
            mockInterval: null
        };

        // Toast Notification Utility
        function showToast(title, message, type = 'success') {
            const toast = document.getElementById('toastNotification');
            const toastCard = document.getElementById('toastCard');
            const toastTitle = document.getElementById('toastTitle');
            const toastMsg = document.getElementById('toastMessage');
            const toastIcon = document.getElementById('toastIcon');

            toastTitle.textContent = title;
            toastMsg.textContent = message;

            if (type === 'danger' || type === 'error') {
                toastCard.className = "bg-white border border-red-200 rounded-xl p-4 shadow-xl flex items-start gap-3";
                toastIcon.className = "ph-fill ph-warning-circle text-2xl text-red-500 shrink-0 mt-0.5";
            } else if (type === 'warning') {
                toastCard.className = "bg-white border border-amber-200 rounded-xl p-4 shadow-xl flex items-start gap-3";
                toastIcon.className = "ph-fill ph-warning text-2xl text-amber-500 shrink-0 mt-0.5";
            } else {
                toastCard.className = "bg-white border border-slate-200 rounded-xl p-4 shadow-xl flex items-start gap-3";
                toastIcon.className = "ph-fill ph-check-circle text-2xl text-emerald-500 shrink-0 mt-0.5";
            }

            toast.classList.remove('translate-y-[-20px]', 'opacity-0', 'pointer-events-none');
            toast.classList.add('translate-y-0', 'opacity-100');

            setTimeout(() => {
                toast.classList.remove('translate-y-0', 'opacity-100');
                toast.classList.add('translate-y-[-20px]', 'opacity-0', 'pointer-events-none');
            }, 4000);
        }

        // ================= RENDER COMPONENTS WITH STATE =================

        function renderEnvironmentCard() {
            // Temp
            const tempEl = document.getElementById('tempValue');
            const tempPill = document.getElementById('tempStatusPill');
            tempEl.textContent = Number(state.temp).toFixed(1);

            if (state.temp >= state.settings.temp_danger) {
                tempEl.className = "text-2xl font-extrabold text-red-600";
                tempPill.className = "mt-2 text-[10px] font-semibold px-1.5 py-0.5 rounded-full bg-red-50 text-red-700 border border-red-200";
                tempPill.textContent = "CRITICAL";
            } else if (state.temp >= state.settings.temp_warning) {
                tempEl.className = "text-2xl font-extrabold text-amber-600";
                tempPill.className = "mt-2 text-[10px] font-semibold px-1.5 py-0.5 rounded-full bg-amber-50 text-amber-700 border border-amber-200";
                tempPill.textContent = "WARNING";
            } else {
                tempEl.className = "text-2xl font-extrabold text-slate-900";
                tempPill.className = "mt-2 text-[10px] font-semibold px-1.5 py-0.5 rounded-full bg-emerald-50 text-emerald-700 border border-emerald-200";
                tempPill.textContent = "NORMAL";
            }

            // Humidity
            const humEl = document.getElementById('humidityValue');
            const humPill = document.getElementById('humStatusPill');
            humEl.textContent = Math.round(state.humidity);

            if (state.humidity >= state.settings.hum_danger) {
                humEl.className = "text-2xl font-extrabold text-red-600";
                humPill.className = "mt-2 text-[10px] font-semibold px-1.5 py-0.5 rounded-full bg-red-50 text-red-700 border border-red-200";
                humPill.textContent = "VERY HIGH";
            } else if (state.humidity >= state.settings.hum_warning) {
                humEl.className = "text-2xl font-extrabold text-amber-600";
                humPill.className = "mt-2 text-[10px] font-semibold px-1.5 py-0.5 rounded-full bg-amber-50 text-amber-700 border border-amber-200";
                humPill.textContent = "ELEVATED";
            } else {
                humEl.className = "text-2xl font-extrabold text-slate-900";
                humPill.className = "mt-2 text-[10px] font-semibold px-1.5 py-0.5 rounded-full bg-blue-50 text-blue-700 border border-blue-200";
                humPill.textContent = "OPTIMAL";
            }

            // Light Intensity (0 - 10)
            const lightVal = Math.min(10, Math.max(0, Math.round(state.light)));
            document.getElementById('lightValue').textContent = lightVal;
            document.getElementById('lightProgressBar').style.width = (lightVal * 10) + '%';
        }

        function renderGasSmokeCard() {
            const isDetected = Boolean(state.smoke_detected === true || state.smoke_detected === 1 || state.smoke_detected === "1");
            const badge = document.getElementById('smokeStatusBadge');
            const badgeDot = document.getElementById('smokeBadgeDot');
            const badgeText = document.getElementById('smokeBadgeText');
            const gasIcon = document.getElementById('gasIcon');
            const gasIconBox = document.getElementById('gasIconBox');
            const smokeDesc = document.getElementById('smokeDescription');
            const backdrop = document.getElementById('smokeAlertBackdrop');
            const globalHazard = document.getElementById('globalHazardBanner');
            const ppmText = document.getElementById('smokePpmText');

            if (isDetected) {
                // Red Pulsing DANGER badge
                badge.className = "px-7 py-3 rounded-2xl font-black text-lg tracking-wider flex items-center gap-2.5 bg-red-50 border border-red-300 text-red-700 shadow-md badge-danger-pulse transition-all";
                badgeDot.className = "w-3 h-3 rounded-full bg-red-600 animate-ping";
                badgeText.textContent = "DANGER";

                gasIconBox.className = "w-9 h-9 rounded-xl bg-red-50 border border-red-200 flex items-center justify-center text-red-600";
                gasIcon.className = "ph-fill ph-fire text-xl animate-bounce";
                smokeDesc.innerHTML = `<span class="text-red-700 font-semibold">SMOKE / COMBUSTIBLE GAS DETECTED!</span> Evacuate or isolate the sector immediately.`;
                backdrop.className = "absolute -right-12 -top-12 w-48 h-48 bg-red-200/40 rounded-full blur-2xl pointer-events-none transition-all duration-500";
                ppmText.textContent = "Trigger: ACTIVE (1)";
                ppmText.className = "font-mono text-[11px] text-red-600 font-bold";
                globalHazard.classList.remove('hidden');
            } else {
                // Green SAFE badge
                badge.className = "px-7 py-3 rounded-2xl font-black text-lg tracking-wider flex items-center gap-2.5 bg-emerald-50 border border-emerald-200 text-emerald-800 transition-all shadow-sm";
                badgeDot.className = "w-3 h-3 rounded-full bg-emerald-500";
                badgeText.textContent = "SAFE";

                gasIconBox.className = "w-9 h-9 rounded-xl bg-emerald-50 border border-emerald-200 flex items-center justify-center text-emerald-600";
                gasIcon.className = "ph-bold ph-wind text-xl";
                smokeDesc.textContent = "No flammable gases or dense smoke detected in the monitored enclosure.";
                backdrop.className = "absolute -right-12 -top-12 w-40 h-40 bg-emerald-100/50 rounded-full blur-2xl pointer-events-none transition-all duration-500";
                ppmText.textContent = "Status: Nominal (0)";
                ppmText.className = "font-mono text-[11px] text-slate-500";
                globalHazard.classList.add('hidden');
            }
        }

        function renderValveCard() {
            const isOpen = Boolean(state.valve_open === true || state.valve_open === 1 || state.valve_open === "true");
            const iconBox = document.getElementById('valveIndicatorIconBox');
            const icon = document.getElementById('valveIcon');
            const text = document.getElementById('valveStatusText');
            const chip = document.getElementById('valveStatusChip');
            const lastCmd = document.getElementById('valveLastCmd');

            if (isOpen) {
                iconBox.className = "w-8 h-8 rounded-lg bg-emerald-100 text-emerald-700 flex items-center justify-center";
                icon.className = "ph-bold ph-lock-key-open text-lg";
                text.className = "text-base font-bold text-emerald-700 tracking-wide";
                text.textContent = "OPEN";
                chip.className = "px-2.5 py-1 text-xs font-semibold rounded-lg bg-emerald-50 border border-emerald-200 text-emerald-700";
                chip.textContent = "Active Flow";
                lastCmd.textContent = "Normal Line";
            } else {
                iconBox.className = "w-8 h-8 rounded-lg bg-red-100 text-red-700 flex items-center justify-center";
                icon.className = "ph-bold ph-lock-key text-lg";
                text.className = "text-base font-bold text-red-700 tracking-wide";
                text.textContent = "CLOSED";
                chip.className = "px-2.5 py-1 text-xs font-semibold rounded-lg bg-red-50 border border-red-200 text-red-700";
                chip.textContent = "Shut Off";
                lastCmd.textContent = "Safety Cutoff Active";
            }
        }

        function populateSettingsInputs() {
            if (state.settings) {
                document.getElementById('tempWarningInput').value = state.settings.temp_warning ?? 28;
                document.getElementById('tempDangerInput').value = state.settings.temp_danger ?? 35;
                document.getElementById('humWarningInput').value = state.settings.hum_warning ?? 70;
                document.getElementById('humDangerInput').value = state.settings.hum_danger ?? 85;
            }
        }

        // ================= REAL-TIME DATA SYNC =================

        window.initRealtimeSync = function () {
            if (window.firebaseDB && window.isFirebaseConfigured) {
                console.log("Listening to Firebase realtime nodes: /roomhub/latest and /roomhub/settings");

                // 1. Subscribe to /roomhub/latest
                const latestRef = window.firebaseRef(window.firebaseDB, 'roomhub/latest');
                window.firebaseOnValue(latestRef, (snapshot) => {
                    const data = snapshot.val();
                    if (data) {
                        if (data.temperature !== undefined) state.temp = data.temperature;
                        if (data.temp !== undefined) state.temp = data.temp;
                        if (data.humidity !== undefined) state.humidity = data.humidity;
                        if (data.light_intensity !== undefined) state.light = data.light_intensity;
                        if (data.light !== undefined) state.light = data.light;
                        if (data.smoke_detected !== undefined) state.smoke_detected = data.smoke_detected;
                        if (data.valve_open !== undefined) state.valve_open = data.valve_open;

                        if (sessionStorage.getItem('safehome_auth') !== 'true') {
                            window.location.href = 'login.html';
                            return;
                        }
                        renderEnvironmentCard();
                        renderGasSmokeCard();
                        renderValveCard();

                        // Online/Offline Detection based on data updates
                        clearTimeout(window.offlineTimeout);
                        document.getElementById('connIndicator').classList.replace('bg-red-500', 'bg-emerald-500');
                        document.getElementById('connText').textContent = 'Online';
                        document.getElementById('connText').parentElement.classList.replace('bg-red-50', 'bg-emerald-50');
                        document.getElementById('connText').parentElement.classList.replace('text-red-700', 'text-emerald-700');
                        document.getElementById('connText').parentElement.classList.replace('border-red-200', 'border-emerald-200');

                        window.offlineTimeout = setTimeout(() => {
                            document.getElementById('connIndicator').classList.replace('bg-emerald-500', 'bg-red-500');
                            document.getElementById('connText').textContent = 'Offline';
                            document.getElementById('connText').parentElement.classList.replace('bg-emerald-50', 'bg-red-50');
                            document.getElementById('connText').parentElement.classList.replace('text-emerald-700', 'text-red-700');
                            document.getElementById('connText').parentElement.classList.replace('border-emerald-200', 'border-red-200');
                        }, 15000); // 15 seconds without data change = offline

                    }
                }, (error) => {
                    console.error("Firebase latest sync error:", error);
                    showToast("Sync Error", "Could not reach Firebase database path: /roomhub/latest", "danger");
                });

                // 3. Subscribe to /roomhub/valve_open directly for instant UI feedback
                const valveRef = window.firebaseRef(window.firebaseDB, 'roomhub/valve_open');
                window.firebaseOnValue(valveRef, (snapshot) => {
                    const val = snapshot.val();
                    if (val !== null) {
                        state.valve_open = val;
                        renderValveCard();
                    }
                });
                
                // 2. Subscribe to /roomhub/settings (Auto-populate)
                const settingsRef = window.firebaseRef(window.firebaseDB, 'roomhub/settings');
                window.firebaseOnValue(settingsRef, (snapshot) => {
                    const data = snapshot.val();
                    if (data) {
                        state.settings = {
                            temp_warning: data.temp_warning ?? state.settings.temp_warning,
                            temp_danger: data.temp_danger ?? state.settings.temp_danger,
                            hum_warning: data.hum_warning ?? state.settings.hum_warning,
                            hum_danger: data.hum_danger ?? state.settings.hum_danger,
                        };
                        populateSettingsInputs();
                        renderEnvironmentCard(); // re-evaluate thresholds
                    }
                });

            }
        };
        // ================= EMERGENCY CLOSE VALVE ACTION =================
        window.emergencyCloseValve = async function () {
            const btn = document.getElementById('emergencyValveBtn');
            const originalHTML = btn.innerHTML;

            btn.disabled = true;
            btn.innerHTML = `<svg class="animate-spin -ml-1 mr-2 h-5 w-5 text-white" fill="none" viewBox="0 0 24 24"><circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle><path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8v8H4z"></path></svg> SENDING CUTOFF COMMAND...`;

            try {
                if (window.firebaseDB && window.isFirebaseConfigured) {
                    const controlRef = window.firebaseRef(window.firebaseDB, 'roomhub/valve_open');
                    await window.firebaseSet(controlRef, false);

                    // Also set local optimistically
                    state.valve_open = false;
                    renderValveCard();
                    showToast("VALVE EMERGENCY CLOSE SENT", "Payload { valve_open: false } written to /roomhub", "danger");
                }
            } catch (err) {
                console.error("Emergency valve close error:", err);
                showToast("Command Failed", err.message, "danger");
            } finally {
                setTimeout(() => {
                    btn.disabled = false;
                    btn.innerHTML = originalHTML;
                }, 800);
            }
        };

        // ================= SAVE SETTINGS =================
        window.saveSettings = async function (event) {
            event.preventDefault();

            const saveBtn = document.getElementById('saveSettingsBtn');
            const originalText = saveBtn.innerHTML;

            const newSettings = {
                temp_warning: parseFloat(document.getElementById('tempWarningInput').value),
                temp_danger: parseFloat(document.getElementById('tempDangerInput').value),
                hum_warning: parseFloat(document.getElementById('humWarningInput').value),
                hum_danger: parseFloat(document.getElementById('humDangerInput').value)
            };

            // Validation
            if (newSettings.temp_warning >= newSettings.temp_danger) {
                showToast("Invalid Configuration", "Temp Warning must be lower than Temp Danger threshold.", "warning");
                return;
            }
            if (newSettings.hum_warning >= newSettings.hum_danger) {
                showToast("Invalid Configuration", "Hum Warning must be lower than Hum Danger threshold.", "warning");
                return;
            }

            saveBtn.disabled = true;
            saveBtn.innerHTML = `
        <svg class="animate-spin -ml-1 mr-2 h-4 w-4 text-white" fill="none" viewBox="0 0 24 24">
          <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle>
          <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8v8H4z"></path>
        </svg>
        Saving to Firebase...
      `;

            try {
                if (window.firebaseDB && window.isFirebaseConfigured) {
                    const settingsRef = window.firebaseRef(window.firebaseDB, 'roomhub/settings');
                    await window.firebaseSet(settingsRef, newSettings);
                    state.settings = newSettings;
                    renderEnvironmentCard();
                    showToast("Settings Updated", "All 4 threshold parameters saved to /roomhub/settings on Firebase.", "success");
                }
            } catch (err) {
                console.error("Save settings error:", err);
                showToast("Save Failed", err.message, "danger");
            } finally {
                setTimeout(() => {
                    saveBtn.disabled = false;
                    saveBtn.innerHTML = originalText;
                }, 500);
            }
        };

        // Manual reload trigger
        window.loadSettingsFromFirebase = function () {
            populateSettingsInputs();
            showToast("Settings Reloaded", "Form fields re-populated with current hub thresholds.");
        };

        // Logout handler
        window.handleLogout = function (e) {
            sessionStorage.removeItem('safehome_auth');
            sessionStorage.removeItem('safehome_user');
            window.location.href = "login.html";
        };

        // Check login auth guard (convenience check)
        window.addEventListener('DOMContentLoaded', () => {
            renderEnvironmentCard();
            renderGasSmokeCard();
            renderValveCard();
            populateSettingsInputs();
        });
    