open Either;

type license = {
    name: string,
    spdxId: string
};

type repository = {
    name: string,
    fullName: string,
    stargazerCount: int,
    stargazerUrl: string,
    htmlUrl: string,
    license: option(license)
};

type stargazer = {
    login: string,
    avatarUrl: string
};

module Decode = {
    let license = json =>
        Json.Decode.{
            name: json |> field("name", string),
            spdxId: json |> field("spdx_id", string)
        };
    let repository = json =>
        Json.Decode.{
            name: json |> field("name", string),
            fullName: json |> field("full_name", string),
            stargazerCount: json |> field("stargazers_count", int),
            stargazerUrl: json |> field("stargazers_url", string),
            htmlUrl: json |> field("html_url", string),
            license: json |> optional(field("license", license))
        };
    let stargazer = json =>
        Json.Decode.{
            login: json |> field("login", string),
            avatarUrl: json |> field("avatar_url", string)
        };
};

let nextPageUrl : Fetch.Headers.t => option(string) = hdrs => {
    let regexp = [%bs.re "/<([^>]+?)>; rel=\"next\"/"];
    hdrs
    |> Fetch.Headers.get("Link")
    |> Js.Option.andThen((.links) =>
        regexp
        |> Js.Re.exec(links)
        |> Js.Option.andThen((.m)=>Js.toOption(Js.Re.captures(m)[1])));
}

type fetchResponse ('a) = either(Js.Promise.error, (option(string), 'a));

/***
 * `decodeResponse` decodes a `Fetch.Response` into an `fetchResponse`
 * 
 * Should never throw, instead it returns a `Left` value with the error
 */
let decodeResponse = f => promise => {
    open Js.Promise;
    promise
    |> then_(resp => {
        let next = nextPageUrl(Fetch.Response.headers(resp));
        Fetch.Response.text(resp)
        |> then_ (text =>
            text
            |> Js.Json.parseExn
            |> f
            |> (v => resolve (Right ((next, v)))));
    })
    |> catch (err => resolve(Left(err)));
};

let orgReposUrl = orgname =>
    {j|https://api.github.com/orgs/$orgname/repos|j};

type getRepositories = string => Js.Promise.t(fetchResponse(array(repository)));

type getStargazers = string => Js.Promise.t(fetchResponse(array(stargazer)));

let fetchOrgRepositories: getRepositories = url =>
    Fetch.fetch(url) |> decodeResponse(Json.Decode.(array(Decode.repository)));

let fetchStargazers: getStargazers = url =>
    Fetch.fetch(url) |> decodeResponse(Json.Decode.(array(Decode.stargazer)));


type client = {
    fetchOrgRepositories: getRepositories,
    fetchStargazers: getStargazers
}

let defaultClient = {
    fetchOrgRepositories: fetchOrgRepositories,
    fetchStargazers: fetchStargazers
};