import re

code_template = """
const enum SCRATCH_opcode codeTemplate[] = {
    SCRATCH_push, 1, 0x41, 0xff,
    SCRATCH_push, 1, 0x0, 0x0,
    SCRATCH_motionGoto,
    SCRATCH_push, 1, 0xa, 0x0,
    SCRATCH_setVar, 0x0, 0x0,
    SCRATCH_push, 0x1, 0x0, 0x0,
    SCRATCH_setVar, 0x1, 0x0,
    // 1:
    SCRATCH_fetchInput, 0x0, 0x0,
    SCRATCH_jumpIfNot, LABEL2, 0x0,
    SCRATCH_loadVar, 0x0, 0x0,
    SCRATCH_incVar, 0x1, 0x0,
    // 2:
    SCRATCH_fetchInput, 0x2, 0x0,
    SCRATCH_jumpIfNot, LABEL3, 0x0,
    SCRATCH_push, 1, 0x0, 0x0,
    SCRATCH_loadVar, 0x0, 0x0,
    SCRATCH_sub,
    SCRATCH_incVar, 0x1, 0x0,
    // 3:
    SCRATCH_push, 1, 0x41, 0xff,
    SCRATCH_loadVar, 0x1, 0x0,
    SCRATCH_goto,
    SCRATCH_loopJump, LABEL1, 0x0,
};
"""

# Step 1: Split into lines and track element indices
lines = code_template.splitlines()
label_to_index = {}
current_index = 0

# First pass: record label positions
for line in lines:
    line_stripped = line.strip()
    if not line_stripped or line_stripped.startswith("const enum"):
        continue

    label_match = re.match(r"//\s*(\d+):", line_stripped)
    if label_match:
        label_num = label_match.group(1)
        label_to_index[f'LABEL{label_num}'] = current_index
        continue

    # Count elements on this line
    parts = [p.strip() for p in line_stripped.split(',') if p.strip()]
    current_index += len(parts)

# Second pass: replace LABELs while keeping formatting
def replace_labels_in_line(line):
    def repl(match):
        label = match.group(0)
        return str(label_to_index[label])
    return re.sub(r'LABEL\d+', repl, line)

new_lines = [replace_labels_in_line(line) for line in lines]
new_code = "\n".join(new_lines)

print(new_code)
