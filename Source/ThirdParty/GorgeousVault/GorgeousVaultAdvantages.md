# Gorgeous Vault — Competitive Advantages

> **Purpose:** Marketing reference — all advantages of the Gorgeous Vault protection system, organized by audience.

---

## Player Advantages

### 1. No Always-Online DRM — Ever

Unlike SimCity, Diablo III (launch), or Ubisoft's always-online titles, games protected by GorgeousVault **never require an internet connection to play.** The Shipping License Token (SLT) is baked into the binary at packaging time. There is no license check server, no heartbeat, no "checking license..." screen, no phone-home.

- Server goes down? **Your game still works.**
- Bad internet? **Your game still works.**
- Publisher decommissions servers in 5 years? **Your game still works.**
- Single-player campaign? **Zero network calls. Period.**

> *"Traditional DRM asks 'should I let you play?' Gorgeous Vault asks nothing — it just works."*

### 2. Zero Performance Impact From Protection

The SLT is a 512-byte block per plugin, pre-computed at packaging time. There are no runtime decryption passes, no license file reads, no network calls. Protection constants are embedded directly in the executable.

- No frame drops from license checks
- No loading screen delays
- No background processes phoning home
- No "DRM overhead" — the constants just exist in memory like any other game data

### 3. Invisible to Legitimate Players

Legitimate players never see, touch, or interact with the protection system. There are no license dialogs, no activation screens, no serial key prompts, no online registration. The game works from the first launch, identically to an unprotected game.

- No activation limits (unlike Denuvo's machine-limit system)
- No "too many activations" lockouts
- No deauthorization needed when switching machines

### 4. Cheaters Can't Ruin Your Multiplayer Experience

The Multiplayer Integrity Partition cryptographically isolates cracked or modified clients from legitimate players. Every replicated packet is HMAC-signed with keys derived from the developer's license. Cracked clients produce **wrong keys → wrong signatures → packets silently rejected.**

- Cracked players can't join your lobbies
- No cheated values reach the server
- No aimbots, wallhacks, or value manipulation via modified Vault DLLs
- Cracked players experience "bad netcode" — they're isolated without even knowing why

> *"You never have to worry about facing a cheater who pirated the game. They literally cannot connect to you."*

### 5. No Anti-Tamper Time Bombs

Some DRM systems (e.g., Denuvo) are eventually removed by publishers — often years after launch — because they degrade over time or create compatibility issues. Gorgeous Vault's protection is a permanent, non-degrading part of the binary. There's nothing to "remove later" and no performance cliff.

- No "DRM removal patch" needed
- No compatibility issues with future OS updates (constants don't interact with the OS)
- The game you buy on day one works identically on day one thousand

### 6. Your Game Won't Die When Servers Shut Down

The #1 fear with always-online DRM: server shutdown = game death. Gorgeous Vault has **zero server dependency at runtime.** The SLT is permanent. Even if Simsalabim Studios ceased to exist tomorrow, every game packaged with a valid SLT continues working forever.

> *"Your purchase is truly yours. No strings attached."*

### 7. Privacy Respected by Design

Gorgeous Vault collects **zero data** from players in packaged/Shipping builds:

- No telemetry
- No analytics
- No crash reports
- No network calls
- No background processes
- No player fingerprinting

All data collection happens exclusively during the developer's Editor workflow (license validation) — never at the player level.

---

## Developer Advantages

### 8. Protection IS Functionality — Not a Wrapper

Traditional DRM wraps your code in a protective layer that can be peeled off. Gorgeous Vault weaves License-Derived Computation Constants (LDCCs) directly into the OV serialization, AR replication, caching, hashing, and notification subsystems.

- There is no `if (licensed)` branch to NOP
- There is no "check" to bypass
- Strip the protection → strip the product
- **The DRM IS the product.**

> *"You can't remove the lock from the door because the lock IS the door."*

### 9. No "Crack Once, Crack Forever" — Cheating Is a Recurring Cost

Traditional DRM is cracked once and the crack works forever. Gorgeous Vault's Epoch Key rotation means LDCCs change on a configurable schedule:

| Epoch Length | Annual Crack Cost vs. Traditional AC |
|---|---|
| 365 days | ~1× (comparable to traditional) |
| 90 days (default) | ~4× more expensive |
| 30 days | ~12× more expensive |
| 5 days (esports) | ~73× more expensive |

Every rotation invalidates all existing cracks for multiplayer. The cracker must reverse-engineer new LDCCs every epoch — the same expensive work, repeated indefinitely.

> *"Every other anti-cheat is a one-time wall. Gorgeous Vault is a treadmill that never stops."*

### 10. Multiplayer Integrity Without Always-Online DRM

Cracked Editor DLLs cannot join legitimate Shipping games. Different Vault → different LDCCs → different HMAC keys → packets rejected. No special DRM check needed — **the protocol itself is the enforcement.**

This eliminates the need for:
- Always-online license verification (SimCity, Ubisoft titles)
- Server-side ownership checks at connection time
- Periodic "heartbeat" pings during gameplay
- Any DRM infrastructure on your game servers

> *"Your players get the fairness of server-side validation without the fragility of server-side DRM."*

### 11. Anti-Cheat Gets Stronger With Every Gorgeous Plugin

Per-Plugin Key Spread (§7.5) means each Gorgeous plugin generates its own independent SLT compartment with independent keys:

| Project Configuration | Crack Cost |
|---|---|
| GorgeousCore only | Baseline (1×) |
| + GorgeousEvents | 2× |
| + GorgeousEntertaining | 3× |
| + N plugins | N× |

Cross-compartment binding prevents selective stripping — crack one, and everything dies. Every plugin you add **automatically strengthens your entire anti-cheat posture** at no extra configuration cost.

> *"No other plugin ecosystem makes your anti-cheat STRONGER the more plugins you use from the same vendor."*

### 12. Artists and Designers Never Need a License

The Usage Tier system (T0/T1/T2) automatically classifies team members:

- **T0 (Non-User):** Never opened the project. No license needed.
- **T1 (Indirect User):** Designers and artists who interact with OV-powered features but don't create/modify them. **No license needed.**
- **T2 (Active Developer):** Creates OV types, modifies properties, uses AR API. License required.

Only T2 developers need per-seat licenses. Your art team, level designers, QA testers, and producers use the project for free.

> *"Your 50-person studio might only need 3 Gorgeous licenses."*

### 13. Fair, Transparent Team Detection

The License Classification Algorithm (§6.26.8) uses 7 behavioral signals to distinguish:

| Classification | Description | Enforcement |
|---|---|---|
| **SINGLE_DEV** | One person on multiple machines | None — legitimate use |
| **TEAM** | Multiple developers sharing seats | Graduated warnings (51 days before impact) |
| **PIRACY** | Unauthorized redistribution | Immediate revocation |

- No false positives for single developers with multiple machines
- 51-day grace period for teams to buy additional seats
- Pirates caught via project diversity, machine growth rate, fingerprint churn, and geographic spread
- Classification thresholds are server-side — never exposed to attackers

### 14. The Most Generous Enforcement Timeline in the Industry

When sharing is detected, enforcement is graduated — not immediate:

```
Day 0:   Sharing detected
Day 0-14:   Soft warning (toast in editor, once per session)
Day 14-21:  Hard warning (persistent banner, email to admin)
Day 21:     Renewal rejection begins
Day 21-51:  Existing keys still work (natural expiry window)
Day 51+:    Cascading death (if still unresolved)
```

**51 days** from first detection to any real impact. Full OV/AR functionality throughout the warning period. Recovery is instant — buy the missing seats and you're back in business within minutes.

### 15. GorgeousCore Is Always Free — No License Needed

GorgeousCore's Object Variable and Auto Replication functionality is 100% free, forever. No license, no network calls, no DRM. Full source headers. Full IntelliSense. Full Blueprint support.

Paid licenses only apply to sibling plugins (GorgeousEntertaining, GorgeousEvents, etc.). And even those only require a license during development — players never interact with licensing.

### 16. Free Anti-Cheat for Core-Only Projects

GorgeousCore-only projects can opt into a **free, auto-provisioned Anti-Cheat Core License** that provides full cryptographic protection in Shipping builds:

- No payment, no trial, no expiration — free forever
- Same security as paid licenses (project-bound, hardware-bound, epoch-rotating)
- Opt-in only (respects the zero-network-call principle for those who don't want it)

> *"Anti-cheat for everyone. Even if you've never paid us a cent."*

### 17. Offline-First — Your License Is Truly Yours

The entire licensing system is designed to work offline:

- 365-day offline JWT with hardware binding
- Deterministic epoch chain self-renewal via HMAC-SHA256 — works forever without a server
- BIP-39 recovery phrase (24-word mnemonic) for hardware migration — fully offline
- Auto-stored fallback key survives up to 365 days of server outage
- No packaging budget, no limited-use tokens, no "you must be online to package"

> *"Your license works on a submarine. On the ISS. In a bunker after the apocalypse. It works everywhere, forever."*

### 18. Full Transparency — See Everything We See

The License Transparency Dashboard (§6.30) shows developers **every data point** the server stores:

- **My Licenses:** Status, epoch, classification, team binding
- **Hardware Fingerprints:** Every registered machine with friendly names, add/remove/rename
- **Usage Statistics:** Exact numbers used by the classification algorithm, per machine, exportable as CSV
- **Data & Privacy:** Complete data inventory, retention periods, export, delete, anonymize

> *"If we collect it, you can see it. There is nothing behind the curtain."*

**Privacy guarantees:**
- Hardware fingerprints are SHA-256 hashed client-side — we never see raw MAC/CPU/hostname
- IP addresses are never stored — only country code derived at request time, then discarded
- Full JSON data export at any time
- GDPR-aligned deletion (72-hour processing, irreversible, includes all backups within 30 days)
- Zero data collection in packaged/Shipping builds

### 19. The Rot — Psychological Warfare Against Cheaters

When cheating is detected, Mode 0 ("The Rot") doesn't ban, kick, or display an error. It **silently corrupts the cheater's game state over time:**

- Phase 1 (0-30s): Normal behavior — cheater thinks the cheat is working
- Phase 2 (30-90s): Subtle corruption — occasional wrong values, minor desyncs
- Phase 3 (90-180s): Escalating — visible gameplay degradation, broken abilities
- Phase 4 (180s+): Terminal — game is unplayable; save data poisoned

The corruption is channeled through LDCC perturbation — **there is no separate "corruption code" for the cheater to find and disable.** The cheater can't tell when detection happened, what triggered it, or how to reverse it.

> *"The best anti-cheat doesn't ban cheaters. It makes them wonder if their own cheat is broken."*

### 20. Configurable Response — Your Game, Your Rules

Three anti-cheat response modes, baked into the SLT so cheaters can't downgrade:

| Mode | Behavior | Best For |
|---|---|---|
| **0 — The Rot** | Silent corruption, no server notification | Psychological deterrence, single-player |
| **1 — Kick + Callback** | Immediate disconnect + `OnAntiCheatViolation` delegate | Competitive games, esports |
| **2 — Rot + Delayed Callback** | Silent corruption first, then server notification after 30-120s | Most multiplayer games (recommended) |

Mode 2 gives maximum information: the cheater experiences silent degradation (can't pinpoint detection) AND the server gets informed for persistent bans.

### 21. Industry-First: Serialization-Layer Anti-Cheat

No existing anti-cheat — EAC, BattlEye, Vanguard, VAC, Denuvo Anti-Cheat — provides per-variable cryptographic integrity with a license-derived, epoch-rotating key at the serialization layer. This is genuinely novel.

Traditional anti-cheat operates at the process/memory level. Gorgeous Vault operates at the **data replication level** — where it actually matters for multiplayer integrity. Both layers complement each other:

| Layer | Gorgeous Vault | Traditional AC (EAC/BattlEye) |
|---|---|---|
| Memory protection | OV Integrity Canary (post-API edits) | Game memory (pre-API edits) |
| Wire protection | HMAC-signed packets | — (not provided) |
| Binary integrity | Accumulator chain, `.text` hash | Signature verification |
| Kernel-level | — (not provided) | Driver-level monitoring |

> *"We don't replace EAC — we fill the gap EAC can't reach."*

### 22. Standard UE Developer Settings — Zero Learning Curve

All Vault configuration is exposed through Unreal Engine's standard Developer Settings:

- `Project Settings → Gorgeous Core → Licensing`
- Familiar `UPROPERTY` interface with tooltips, clamp ranges, and `EditCondition` visibility
- Open-source settings class — inspect, modify, version-control in your project's `.ini` files
- Smart auto-detection: paid plugins present? AC auto-implied. Core-only? AC opt-in shown.

### 23. Explicit AC Disable — Your Choice, Always

For projects that use paid Gorgeous plugins but don't need anti-cheat (single-player, tools, non-game applications), `bExplicitlyDisableAntiCheat` cleanly removes the entire AC layer:

- All OV/AR functionality preserved — only the AC armor is removed
- Hides all AC sub-settings (nothing to configure)
- Per-project — one studio can have AC enabled on their competitive title and disabled on their narrative game
- Reversible — uncheck and repackage

### 24. Open Architecture — Open Headers, Closed Core

All `.h` headers are open-source. Full IntelliSense, full documentation, full code navigation. Only the Vault's internal implementation (~7,978 lines of core OV + AR code) is compiled into a prebuilt DLL.

- Developers can read every API, every type, every parameter
- No "black box" APIs — the interface is completely transparent
- Thin proxy pattern — zero overhead forwarding from open-source code to Vault
- ~95 C-linkage exports across all subsystems

### 25. Cascading Death Is Recoverable — Not Destructive

When a license issue is detected:

- OV/AR functionality is disabled, but the Editor **never crashes**
- Clear messaging identifies exactly which plugin has the problem
- Recovery is instant — fix the license issue and functionality returns without an Editor restart
- Single `CachedFunctions.Empty()` call — clean, reversible, no data loss

### 26. Fab & Epic Nova Compliance Out of the Box

- Third-party library precedent (like FMOD, Vivox, Steamworks SDK)
- All UBT module source shipped — only the Vault DLL is proprietary
- Full disclosure in Fab listings
- License management operations are **independent of Epic Nova EULA**
- Zero hidden telemetry, zero silent data collection
- Network calls require explicit consent dialog before first use

### 27. Strictly More Secure Than Always-Online DRM

Online DRM has one critical weakness: its entire security model reduces to **a single yes/no check.** Find the branch, NOP it, done. Or emulate the server response. Or run a private server. One person does this once → crack shared with millions → full, permanent, identical access.

Gorgeous Vault has no yes/no check. There is no branch. The cryptographic material **is** the functionality.

```
ONLINE DRM:
  Client → "Am I allowed to play?" → Server → "Yes"
  Crack: Patch one branch, or fake the server response. Done forever.

GORGEOUS VAULT:
  SLT → EpochKey → 12 LDCCs → woven into every function
  Crack: Reverse-engineer an obfuscated, VM-protected DLL to extract
         3072 bits of key material. Repeat every epoch rotation.
```

**Direct comparison:**

| Property | Online DRM | Gorgeous Vault |
|---|---|---|
| What must be defeated | A single yes/no check | 12 interleaved cryptographic constants (3072 bits) |
| Attack complexity | Find one branch, NOP it | Reverse-engineer obfuscated, VM-protected DLL |
| Crack durability | **Permanent** | Expires at epoch rotation (configurable: 5–365 days) |
| Server emulation | Trivial — fake "yes" response | **Impossible** — no runtime server to emulate |
| Private servers | Common crack strategy | Meaningless — no server dependency |
| Binary patch | One instruction (NOP the check) | No branch to patch — wrong LDCCs → wrong math → silent corruption |
| Crack verification | Easy — "does it launch?" | **Extremely hard** — The Rot means it *appears* to work, then silently degrades |

**For multiplayer — the gap is massive:**
- Online DRM: Once cracked, the client has full unmodified functionality. Connects to legitimate servers, plays with legitimate players, indistinguishable from a real copy. Protection evaporates entirely.
- Gorgeous Vault: Without correct LDCCs, every packet has wrong HMACs → silently rejected by all legitimate peers/servers. There is no "bypass the check and play normally" path.

**For single-player — still stronger:**
- Online DRM's biggest weakness is server emulators — a tiny program that always responds "yes." There's nothing to emulate with Gorgeous Vault.
- The Rot makes crack verification extremely difficult. With online DRM, "does it launch?" = verified. With Vault, the cracker might have wrong LDCCs and not realize it for hours because corruption is silent and gradual.

> *"The irony: the system that's more secure is also the one that's more player-friendly. Online DRM sacrifices user experience for a weaker form of security. Gorgeous Vault provides stronger security with zero user-facing cost."*

---

## Comparison Summary

| Feature | Gorgeous Vault | Traditional DRM (Denuvo) | Traditional AC (EAC) | Always-Online (SimCity) |
|---|---|---|---|---|
| Always-online required | **No** | No (but machine limits) | No | **Yes** |
| Player-visible | **Never** | Activation limits visible | Splash screen / driver | Connection screens |
| Survives server shutdown | **Yes (forever)** | Depends on publisher | Yes | **No** |
| Performance overhead | **Zero** | Measurable (CPU) | Measurable (kernel) | Network latency |
| Crack cost model | **Recurring (epoch)** | One-time | One-time | N/A (server-side) |
| Protects replication layer | **Yes (HMAC)** | No | No | Server-side only |
| Per-variable integrity | **Yes (Canary)** | No | No | No |
| Scales with more plugins | **Yes (Key Spread)** | No | No | No |
| Free tier available | **Yes (Core + free AC)** | No | No | No |
| Full data transparency | **Yes (Dashboard)** | No | No | No |
| Offline-forever support | **Yes** | Limited | Yes | **No** |
| GDPR-compliant data controls | **Yes** | Varies | Varies | Varies |
| Strictly more secure | **Yes** | Comparable (single-player) | Different layer | **No** (weakest model) |
| Security scales with adoption | **Yes (more OVs = more coverage)** | No | No | No |

---

## The Scaling Principle — Security That Rewards Good Development

**The single most important thing to understand about Gorgeous Vault:** protection strength is not a fixed property of the system — it scales directly with how the developer uses it.

### What "Scales" Means

Every game variable that the developer stores in an Object Variable (OV) instead of a raw `UPROPERTY` gains:
- Per-variable HMAC Canary (memory edit detection)
- LDCC-derived serialization (cracked binaries produce wrong output)
- Multiplayer Integrity Partition (cracked packets rejected)
- The Rot trigger on tamper (silent behavioral corruption)

A variable stored outside OVs? Unprotected. Gorgeous Vault can't see it.

```
SECURITY AS A FUNCTION OF OV ADOPTION:

  0% of critical state in OVs  → 0% protection  (Vault has nothing to protect)
  30% in OVs (health, currency) → moderate protection (core economy safe)
  70% in OVs (+ cooldowns, buffs) → strong protection (most cheat vectors closed)
  100% in OVs (all replicated state) → maximum protection (every variable armored)

  The developer CHOOSES their security posture through architecture decisions.
```

### What "Rewards Good Development" Means

The conditions for maximum security are the **same conditions for a well-maintained game:**

| Good Practice | Security Benefit |
|---|---|
| Store critical state in OVs | Every variable gets cryptographic armor |
| Ship regular game updates | Epoch rotation invalidates cracks (new SLT baked per build) |
| Use Auto Replication for netcode | Every packet gets HMAC integrity — Multiplayer Partition active |
| Add more Gorgeous plugins | Key Spread multiplies crack cost (N plugins = N× cost) |
| Configure epoch length for your game type | Tighter rotation = higher recurring cost for crackers |

**The inverse is also true:** a game that stores everything in raw UPROPERTYs, never ships updates, and uses custom netcode instead of AR gets minimal benefit. The system doesn't magically protect what it can't see.

### Why This Is an Advantage (Not a Limitation)

Traditional DRM is binary: it's either on or off, and the developer has no control over its strength. Gorgeous Vault gives the developer a **dial:**

- Prototype / jam game? Use OVs for convenience, get some protection for free. Don't think about security.
- Competitive multiplayer? Move all critical state to OVs, enable Canary, set 30-day epochs, use AR for replication. Maximum armor.
- Live-service MMO? Same as above + multiple Gorgeous plugins + 5-day epochs for ranked modes. Every update resets the clock for crackers.

The developer is in control. The system scales to match their investment.

> *"Gorgeous Vault doesn't ask you to bolt on security as an afterthought. It makes security a natural consequence of using the right architecture. The more you use OVs, the more you're protected — automatically, for free, with zero additional configuration."*

### The Compound Flywheel

```
Developer adopts OVs for convenience (good API, Blueprint support, Auto Replication)
   → Critical state now flows through the Vault
   → Every OV variable is automatically HMAC-protected
   → Developer ships update (new features, bug fixes — normal game maintenance)
   → New SLT baked with rotated epoch key
   → All existing cracks invalidated
   → Cheaters must re-crack (same cost, recurring)
   → Meanwhile developer adds GorgeousEvents for gameplay features
   → Key Spread doubles the crack cost (2 independent compartments)
   → Developer's security posture keeps strengthening automatically
   → Without the developer ever thinking about "anti-cheat"
```

The developer never configures a firewall rule, never writes a validation check, never integrates an anti-cheat SDK. They just use OVs, ship updates, and add plugins. **The security is a side effect of building the game.**

---

*Document version: 1.2 — February 28, 2026*
*Source: GorgeousProtectionPlan.md (Gorgeous Vault Technical Protection Plan)*
