# Customer Card Visual Comparison

## BEFORE: Original Card Design

```
┌────────────────────────────────────────────────────────┐
│ [S]  ╭───╮                                          [+]│
│ [T]  │ A │  CUSTOMER NAME                             │
│ [R]  ╰───╯  #CariKod123                               │
│ [I]          📞 555-1234567                            │
│ [P]          📧 customer@example.com                   │
│ [E]                                                    │
│                                          [AKTIF]       │
└────────────────────────────────────────────────────────┘

Features:
- Single Add button (+)
- Email display
- Simple reference number
- No ID security
```

## AFTER: Improved Card Design

```
┌────────────────────────────────────────────────────────┐
│ [S]  ╭───╮                                  [+][✎][👁]│
│ [T]  │ A │  CUSTOMER NAME                             │
│ [R]  ╰───╯  ID: CariKod123 (Prominent Blue)           │
│ [I]          TC: 123******89 (Masked)                  │
│ [P]          📞 555-1234567                            │
│ [E]                                                    │
│                                          [AKTIF]       │
└────────────────────────────────────────────────────────┘

New Features:
✅ Three action buttons: Add, Edit, ID Toggle
✅ Prominent Database Customer ID (Blue)
✅ Masked TC Kimlik by default
✅ Click eye icon to reveal full ID
✅ Direct Edit access
✅ Removed email to reduce clutter
✅ Enhanced security
```

## Card Anatomy - Detailed View

```
Top Section:
┌────────────────────────────────────────────────────────┐
│ [4px Status Strip - Color coded]                      │
│                                                        │
│    ╭───────╮                                           │
│    │   A   │  <-- Avatar (40x40px)                    │
│    │       │      Shows customer initial              │
│    ╰───────╯      Bordered with status color          │
│                                                        │
└────────────────────────────────────────────────────────┘

Action Buttons (Top Right):
┌────────────────┐
│  [+]  [✎]  [👁]│  <-- 22px each, 4px spacing
│                │
│  Add  Edit  ID │  <-- Function labels
└────────────────┘

Information Display:
┌────────────────────────────────┐
│ CUSTOMER NAME (11pt, Bold)     │  <-- Primary text
│ ID: CariKod123 (9pt, Blue)     │  <-- Database ID (NEW)
│ TC: 123******89 (8pt, White)   │  <-- Masked ID (NEW)
│ 📞 555-1234567 (9pt, White)    │  <-- Phone
└────────────────────────────────┘

Status Badge (Bottom Right):
┌──────────────┐
│   [AKTIF]    │  <-- Pill-shaped badge
└──────────────┘     18px height
                     Color: Status dependent
```

## Button States Visual

### Normal State (Not Hovering)
```
  ◯      ◯      ◯    <-- Ghost circles (30% opacity)
  +      ✎      👁   <-- Icons (30% opacity)
```

### Hover State
```
  ⬤      ⬤      ⬤    <-- Filled circles (100% opacity)
  +      ✎      👁   <-- Icons (100% opacity)
 Add    Edit    ID   <-- Implied function
```

### Click Actions
```
[+]  → Opens Quick Add Menu
      ├─ 🏠 Add Home
      ├─ 🚗 Add Car
      ├─ 🏞️ Add Land
      ├─ 🏰 Add Villa
      ├─ 🚜 Add Field
      └─ 🏢 Add Commercial

[✎]  → Opens Edit Customer Dialog
      (CCustomerDialog in IUPDATEUSER mode)

[👁] → Toggles ID Masking
      Masked:   123******89
      Unmasked: 12345678901
```

## ID Masking States

### Masked (Default - Secure)
```
TC: 123******89
    ↑↑↑      ↑↑
    First 3  Last 2
    (6 asterisks in middle)
```

### Unmasked (After Click)
```
TC: 12345678901
    (Full 11-digit ID visible)
```

### Toggle Animation
```
Click [👁]
    ↓
Card Refreshes
    ↓
ID State Changes
    ↓
Visual Update Complete
```

## Color Palette

### Card Background (Dark Theme)
```
┌─────────────────────┐
│ ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓  │ Top: RGB(50,50,50)
│ ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓  │ Gradient
│ ████████████████    │ Bottom: RGB(40,40,40)
└─────────────────────┘
```

### Selected State
```
┌─────────────────────┐
│ ░░░░░░░░░░░░░░░░░  │ Top: RGB(60,65,80)
│ ░░░░░░░░░░░░░░░░░  │ Blue tint
│ ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒    │ Bottom: RGB(45,50,60)
└─────────────────────┘
Border: RGB(64,158,255) 2px Blue glow
```

### Text Colors
```
Primary Text:   ████████  RGB(255,255,255) White
Secondary Text: ▓▓▓▓▓▓▓▓  RGB(150,150,150) Light Gray
Accent (DB ID): ░░░░░░░░  RGB(100,180,255) Blue
```

## Status Color Examples

```
[RANDEVU]     Purple    RGB(155,89,182)
[TEKLİF]      Green     RGB(46,204,113)
[SICAK ALICI] Red       RGB(231,76,60)
[AKTİF]       Blue      RGB(64,115,255)
[PASİF]       Gray      RGB(189,195,199)
```

## Spacing & Measurements

```
Card Dimensions:
- Width:  270px (DPI scaled)
- Height: 130px (DPI scaled)
- Padding: 12px (DPI scaled)
- Radius: 8px rounded corners

Button Specifications:
- Size: 22x22px
- Spacing: 4px between buttons
- Margin from edge: 12px

Text Line Spacing:
- Name to ID: 18px
- ID to TC: 16px
- TC to Phone: 16px

Status Stripe:
- Width: 4px
- Height: Full card height
- Position: Left edge
```

## Hover Effects

### Card Hover
```
Normal:  [Card with shadow]
Hover:   [Card with brighter shadow]
         [Buttons visible 100%]
         [No selection border]

Selected: [Card with blue border]
          [Buttons visible 100%]
          [Background: blue tint]
```

### Button Hover
```
Button State Transition:
Not Hovering → Hovering → Clicked
   30%     →    100%   →  Action
  Ghost    →   Solid   →  Execute
```

## Implementation Summary

### Key Changes:
1. ✅ Three buttons instead of one
2. ✅ ID masking for security
3. ✅ Prominent DB Customer ID
4. ✅ Removed email (space saving)
5. ✅ Enhanced visual hierarchy
6. ✅ Better information organization

### User Benefits:
- 🔒 Enhanced Security (masked IDs)
- ⚡ Faster Workflow (direct edit)
- 👁️ On-demand ID viewing
- 🎯 Clear identification (DB ID)
- 🎨 Modern, professional look
- 📱 Optimized space usage

---

**Note**: All measurements are DPI-aware and scale automatically with display settings.
