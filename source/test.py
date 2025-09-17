
s = "! !! !!!!! !! !!!!!  !!!!  !!!!  !  !  !  !!!!!!! !! !! !!!!! !! !!!!!!!! !!!!! !!! ! !! !!! ! !! !! !!! "
    "! !! !!!!! !! !!!!!  !!!!  !!!!  !  !  !  !!!!!!! !! !! !!!!! !! !!!!!!!! !!!!! !!! ! !! !!! ! !! !! !!! "

# break into chunks of 3
chunks = [s[i:i+3] for i in range(0, len(s), 3)]

output_lines = []
for i, chunk in enumerate(chunks, 1):
    output_lines.append(chunk)
    if i % 7 == 0:
        output_lines.append("")

output_lines_str = "\n".join(output_lines)
print(output_lines_str)
