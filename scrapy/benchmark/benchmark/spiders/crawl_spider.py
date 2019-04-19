import scrapy
from scrapy.spiders import CrawlSpider, Rule
from scrapy.linkextractors import LinkExtractor

class TestSpider(CrawlSpider):
    name = "testspider"
    start_urls = [
        'http://localhost/',
        'http://testing-ground.scraping.pro/',
        'http://webscraper.io/test-sites/e-commerce/allinone'
    ]
    allowed_domains = ['localhost', 'testing-ground.scraping.pro', 'webscraper.io']

    rules = (
        Rule(LinkExtractor(allow=())),
    )

    def parse_item(self, response):
        print response.url
