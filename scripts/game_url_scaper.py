# Cannot be run on the zoo due to selenium dependancy on desktop browser driver
from selenium import webdriver
from bs4 import BeautifulSoup
import datetime as dt


class GameUrlScraper:
    def __init__(self, base_url, run=True):
        self.base_url = base_url
        self.run = run

        # Set up the headless chrome browser to handle js loading on db page
        driver_path = "/Users/ryanjcain 1/Desktop/Yale/Spring 2020/Games/shogi/venv/lib/chromium-browser/chromedriver"
        options = webdriver.ChromeOptions()
        options.add_argument('--ignore-certificate-errors')
        options.add_argument('--incognito')
        options.add_argument('--headless')

        if self.run:
            self.log("Launching headless chrome")
            self.driver = webdriver.Chrome(driver_path, options=options)
            self.log("Sucessfully loaded headless chrome driver")

    def get_players(self, filename):
        '''Function to scrape all the player names from the different
        alphebetized pages
        '''
        gyou = ["あ行", "か行", "さ行",
                "た行", "な行", "は行",
                "ま行", "や行", "わ行"]

        # Write all the player urls to a file. Save incrementally in case something breaks!
        f = open(filename, "w")
        for kana in gyou:
            self.log("Getting players from {}".format(kana))

            # Access the page via the chrome driver and save the page source
            self.driver.get(self.base_url + "players/" + kana)
            page_source = self.driver.page_source

            # Parse the page source with BeautifulSoup and get all player links
            soup = BeautifulSoup(page_source, 'xml') 
            for link in soup.select(".list-group-item"):
                f.write(link.get('href') + "\n")
        f.close()

    def get_games(self, source, dest):
        ''' Function to get ALL of the games associated with each player
        '''
        f = open(source, "r")
        lines = f.readlines()
        f.close()

        out = open(dest, "w")
        for line in lines:
            player_url = self.base_url + line.strip()[1:]
            player = player_url.split('/')[-1]
            self.log("Getting games for <{}>".format(player))

            # Get the first page
            self.driver.get(player_url)
            page_source = self.driver.page_source

            # Loop through all of the pages for the player's games
            page = 1
            total_games = 0
            scraping = True
            while scraping:
                # Try accessing the current page, make the soup
                self.driver.get(player_url + "/page/{}".format(page))
                page_source = self.driver.page_source
                soup = BeautifulSoup(page_source, 'xml') 

                # Get all of the game link objects
                links = soup.select(".list-group-item")
                if links:
                    for link in links:
                        out.write(link.get('href')+ "\n")
                else: # Stop scraping, no more content on the page
                    scraping = False

                self.log("Scraped {} games on page {}".format(len(links), page))
                total_games += len(links)
                page += 1

            self.log("Scraped a total of {} games for <{}>".format(total_games, player))

        out.close()


    def log(self, message):
        print("[ShogiScraper {}] : {}".format(dt.datetime.now(), message))


if __name__ == "__main__":
    base_url = "https://shogidb2.com/"
    spider = GameUrlScraper(base_url)

    # Uncomment to get a new copy of players from database
    # spider.get_players("./data/players.txt")

    # Uncomment to get a new copy of each player's games
    # Roughly 112,000 games to collect. Run:
    #       sort games.txt | uniq > games_urls.txt
    # to get all of the unique games: ~60,000 then run:
    # spider.get_games("../data/players.txt", dest="../games.txt")



