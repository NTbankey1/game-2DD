# Skills System — Sức Mạnh Thực Sự Của Claude Code

## Skills là gì?

Skills là những **gói kiến thức chuyên sâu** mà Claude Code tải vào khi cần. Thay vì tôi phải "đoán" cách làm việc với một công cụ hay định dạng file, skills cung cấp cho tôi:

- **Cú pháp chính xác** — không phải suy luận, không phải nhớ
- **Workflow tối ưu** — các bước đã được thiết kế sẵn
- **Cạm bẫy thường gặp** — lỗi mà người mới hay mắc
- **Patterns mẫu** — giải pháp đã được kiểm chứng

Kết quả: **Tôi làm việc nhanh hơn, chính xác hơn, và không cần bạn giải thích context.**

---

## Cơ chế hoạt động

```
Bạn nói: "Tạo một .canvas file cho kiến trúc game"

    ↓

Tôi kiểm tra: có skill nào khớp không?

    ↓

Phát hiện: json-canvas → tải skill ngay lập tức

    ↓

Tôi biết chính xác:
- Cấu trúc JSON canvas spec 1.0
- Cách generate ID 16-char hex
- Các node types: text, file, link, group
- Cách đặt edges, colors, layout
- Validation checklist

    ↓

Bạn nhận được file .canvas hoàn chỉnh, đúng spec
```

**Bạn không cần gọi skills bằng tay.** Tôi tự động phát hiện và kích hoạt chúng dựa trên nội dung yêu cầu của bạn.

---

## 5 Skills Bạn Đang Có

### 1. `obsidian-markdown` — Obsidian Flavored Markdown

**Kích hoạt khi:** bạn nói về `.md` files, wikilinks, callouts, tags, properties, embeds.

**Sức mạnh:**
- Viết wikilinks chính xác: `[[Note]]`, `[[Note|Display]]`, `[[Note#^block-id]]`
- Tạo callouts đủ loại: `> [!note]`, `> [!warning]`, `> [!faq]-` (foldable)
- Properties (frontmatter) với đúng YAML syntax
- Embed hình ảnh, PDF, audio, video: `![[image.png|300]]`
- Comments ẩn: `%%comment%%`
- Math (LaTeX) và Mermaid diagrams
- Tags, footnotes, highlights

**Nếu không có skill này:** Tôi sẽ viết markdown tiêu chuẩn — không Obsidian-specific syntax. Bạn sẽ phải tự sửa.

### 2. `obsidian-bases` — Obsidian Bases (.base files)

**Kích hoạt khi:** bạn nói về Bases, table/card/list views, filters, formulas.

**Sức mạnh:**
- Tạo `.base` file với đúng cấu trúc YAML
- Filters phức tạp: `and`/`or`/`not` nesting
- Formulas: arithmetic, conditional, date math, string formatting
- View types: table, cards, list, map + summaries
- Xử lý Duration type đúng cách (truy cập `.days` trước khi `.round()`)
- YAML quoting rules — tránh lỗi syntax

**Nếu không có skill này:** Tôi sẽ viết Base syntax sai — đặc biệt là Duration math và YAML quoting, hai cái dễ sai nhất.

### 3. `json-canvas` — JSON Canvas (.canvas files)

**Kích hoạt khi:** bạn nói về Canvas, mind maps, flowcharts, `.canvas` files.

**Sức mạnh:**
- Tạo node đúng spec: text, file, link, group
- 16-char hex ID generation
- Edges với `fromNode`/`toNode` + directional arrows
- Color presets `"1"`–`"6"` hoặc hex
- Layout guidelines (spacing, sizing, grid alignment)
- Validation checklist built-in

**Nếu không có skill này:** Tôi sẽ guess cấu trúc JSON — rất dễ sai về ID format, direction arrows, và color presets.

### 4. `obsidian-cli` — Obsidian CLI

**Kích hoạt khi:** bạn muốn tương tác với Obsidian vault từ command line.

**Sức mạnh:**
- Đọc/ghi/search note: `obsidian read/search/create/append`
- Quản lý properties và tasks
- Daily notes operations
- Plugin development workflow:
  - Reload: `obsidian plugin:reload id=my-plugin`
  - Check errors: `obsidian dev:errors`
  - Screenshot: `obsidian dev:screenshot`
  - DOM inspection: `obsidian dev:dom`
  - Console logs: `obsidian dev:console`
  - Run JS: `obsidian eval code="..."`

**Nếu không có skill này:** Tôi sẽ không biết CLI syntax, không thể giúp bạn debug plugin một cách hiệu quả.

### 5. `defuddle` — Web Page to Markdown

**Kích hoạt khi:** bạn đưa URL cần đọc/phân tích.

**Sức mạnh:**
- Extract nội dung chính từ web pages, bỏ navigation/quảng cáo
- Output markdown sạch — tiết kiệm tokens
- So với WebFetch: **cho kết quả sạch hơn, ít noise hơn**

---

## Tại sao đây là Superpower?

### 1. Zero Context Cần Giải Thích

Không cần nói "hãy dùng định dạng Obsidian" hay "theo spec JSON Canvas version 1.0". Tôi đã biết. Bạn chỉ cần nói **muốn gì**, không cần nói **làm thế nào**.

### 2. Chính Xác Đến Từng Chi Tiết

Mỗi skill chứa đựng:
- Edge cases đã được mapping
- Common pitfalls đã được document
- Validation rules có sẵn

Ví dụ: Với `obsidian-bases`, tôi biết:
- ❌ `(now() - file.ctime).round(0)` → **sai** (Duration không support `.round()` trực tiếp)
- ✅ `(now() - file.ctime).days.round(0)` → **đúng**

### 3. Workflow Tối Ưu

Skills không chỉ dạy syntax — chúng dạy **trình tự**. Ví dụ với canvas:
1. Tạo file với base structure
2. Generate unique IDs
3. Add nodes với position phù hợp
4. Add edges
5. Validate

Không skip step. Không phải nhớ.

### 4. Plugin Development Support

Skill `obsidian-cli` biến tôi thành trợ lý Obsidian plugin developer thực thụ:
- Reload → check errors → screenshot → DOM inspect → console log
- Vòng lặp phát triển nhanh, không gián đoạn

### 5. Tiết Kiệm Token, Tăng Tốc Độ

- Defuddle: bỏ navigation/quảng cáo → nội dung sạch → ít token hơn
- Skills: tôi không cần "nghĩ" cách làm → phản hồi nhanh hơn

---

## Khi Nào Skills Được Kích Hoạt?

Skills tự động kích hoạt dựa trên ngữ cảnh. Ví dụ:

| Bạn nói | Skill được kích hoạt |
|---------|---------------------|
| "Tạo note kiến trúc game" | `obsidian-markdown` (+ wikilinks, callouts) |
| "Lọc tasks theo status" | `obsidian-bases` |
| "Vẽ mind map canvas" | `json-canvas` |
| "Mở CLI Obsidian" | `obsidian-cli` |
| "Đọc article này" | `defuddle` |
| "Debug plugin Obsidian" | `obsidian-cli` + `obsidian-markdown` |

Đôi khi **nhiều skill cùng lúc** — tôi xử lý tuần tự theo thứ tự ưu tiên.

---

## Ecosystem Skills Khác

Ngoài 5 Obsidian skills, còn có hơn 20 skills khác trong hệ thống Superpowers:

| Domain | Skills |
|--------|--------|
| **Process** | brainstorming, systematic-debugging, writing-plans, executing-plans, test-driven-development, verification-before-completion, requesting-code-review, receiving-code-review |
| **Development** | subagent-driven-development, dispatching-parallel-agents, finishing-a-development-branch |
| **Chất lượng** | code-review, bug-hunting-review, simplify, security-review |
| **Công cụ** | run, init, commit, commit-push-pr, update-config, fewwer-permission-prompts |
| **Học tập** | deep-research, prompt-engineering-patterns, dataviz, claude-api |

Mỗi skill là một "class" mà tôi đã học. Khi bạn cần, tôi dùng nó.

---

## Cách Cài Thêm Skills

Skills được cài qua plugin marketplace:

```
/plugin marketplace add <repo>
```

Hoặc clone repo vào thư mục skills. Mỗi skill đơn giản chỉ là một file `SKILL.md` với:
- **metadata** (name, description, trigger conditions)
- **knowledge** (syntax, patterns, pitfalls)
- **workflows** (các bước tối ưu)

---

## Tóm Lại

> **Skills biến Claude Code từ "một AI thông minh" thành "một chuyên gia trong domain của bạn".**

Không skills → tôi dùng kiến thức tổng quát (có thể sai, có thể thiếu).
Có skills → tôi dùng kiến thức chuyên sâu, chính xác, đã được kiểm chứng.

5 Obsidian skills này biến project của bạn thành **môi trường Obsidian-aware** — nơi tôi hiểu Obsidian không khác gì một người dùng lâu năm.
