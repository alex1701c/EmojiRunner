import { writeFileSync } from "node:fs";

fetch("https://raw.githubusercontent.com/github/gemoji/master/db/emoji.json")
  .then((response) => response.json())
  .then((data) => {
    const emojisWithId = data.map((emoji, index) => ({ ...emoji, id: index }));
    const mapped = Object.groupBy(emojisWithId, (item) => item.category);
    Object.values(mapped).forEach((emojis) => {
      emojis.forEach((emoji) => {
        emoji.name = emoji.aliases[0].replace(/_/g, " ");
        delete emoji.aliases;
        delete emoji.category;
      });
    });

    writeFileSync("emojis.json", JSON.stringify(mapped, undefined, 4));
  });
