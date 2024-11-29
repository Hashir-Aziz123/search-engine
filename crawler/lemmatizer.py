# lemmatizer.py
from spacy.lang.en import English
import spacy

nlp = spacy.load("en_core_web_sm")

def lemmatize_word(word):
    doc = nlp(word)
    return [token.lemma_ for token in doc][0]

# import spacy
# nlp = spacy.load("en_core_web_sm")
# doc = nlp("Hello, world!")
# print([(token.text, token.pos_) for token in doc])
